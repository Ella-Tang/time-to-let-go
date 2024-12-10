#include <WiFiS3.h>
#include <ArduinoHttpClient.h>
#include <SoftwareSerial.h>
#include <Arduino_JSON.h>
#include "arduino_secret.h"

// Thermal Printer
SoftwareSerial thermalPrinterSerial(10, 11); // TX, RX
// Flame Sensor
const int sensorPin = 3;
// Wi-Fi
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
// Firebase
const char* firebaseHost = "time-to-let-go-default-rtdb.firebaseio.com";
const char* firebaseAuth = "AIzaSyBezCtXu7jDw4I2Y4ASPEYy_5eHAp42-dM";
// HTTP Client
WiFiSSLClient wifiClient;
HttpClient http(wifiClient, firebaseHost, 443);
int status = WL_IDLE_STATUS;
// Last timestamp
String lastTimestamp = "";

void setup() {
  Serial.begin(9600);
  pinMode(sensorPin, INPUT);
  thermalPrinterSerial.begin(9600);
  connectToWiFi();
  fetchAndPrintData();
}

void loop() {
  if (status == WL_CONNECTED) {
    fetchAndPrintData();
    int fireState = digitalRead(sensorPin);
    Serial.println("Fire State: " + String(fireState));
    sendFireStateToDatabase(fireState);
  } else {
    connectToWiFi();
  }
}

void connectToWiFi() {
  while (!Serial);
  Serial.print("Connecting to Wi-Fi...");
  while (status != WL_CONNECTED) status = WiFi.begin(ssid, pass);
  Serial.println("Connected to Wi-Fi!");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
}

void fetchAndPrintData() {
  // Firebase URL
  String firebaseURL = "/userInput.json?auth=" + String(firebaseAuth);
  Serial.println("Sending GET request to: " + firebaseURL);
  // Send GET request
  http.get(firebaseURL);
  int httpResponseCode = http.responseStatusCode();
  String responseBody = http.responseBody();
  // HTTP Response
  Serial.println("HTTP Response Code: " + String(httpResponseCode));
  Serial.println("Response Body: " + responseBody);
  // If get response
  if (httpResponseCode == 200) {
    JSONVar jsonData = JSON.parse(responseBody);
     // Find the latest
    String latestKey = "";
    Serial.println("Keys in JSON:");
    for (int i = 0; i < jsonData.keys().length(); i++) {
      String key = (const char*)jsonData.keys()[i];
      Serial.println("Key: " + key);
      if (key > latestKey) latestKey = key; 
    }
    if (latestKey == "") return;
    Serial.println("Latest Key: " + latestKey);
    // Check if new key
    if (latestKey == lastTimestamp) {
      Serial.println("No new data to print. Last Timestamp: " + lastTimestamp);
      return;
    } 
    // Fetch latest data
    JSONVar latestEntry = jsonData[latestKey];
    lastTimestamp = latestKey;
    // Extract details from latest
    String name = (const char*)latestEntry["userInput"]["name"];
    String time = (const char*)latestEntry["userInput"]["time"];
    String description = (const char*)latestEntry["userInput"]["description"];
    JSONVar feelingsArray = latestEntry["userInput"]["feelings"];
    String feelings = "";
    for (int i = 0; i < feelingsArray.length(); i++) {
      String feeling = (const char*)feelingsArray[i];
      if (feeling.length() > 0) feeling[0] = tolower(feeling[0]); 
      if (i > 0) feelings += ", ";
      feelings += feeling;
    }
    String haiku = (const char*)latestEntry["userInput"]["haiku"];
    // Print to thermal printer
    printToThermalPrinter(name, time, description, feelings, haiku);
  } else {
    Serial.println("Error: Failed to retrieve data from Firebase.");
  }
  http.stop();
}

void printToThermalPrinter(String name, String time, String description, String feelings, String haiku) {
  // Send print status
  sendPrintStatusToDatabase("Started");
  // Heading
  thermalPrinterSerial.write(0x1B); // ESC
  thermalPrinterSerial.write('a'); // Alignment
  thermalPrinterSerial.write((uint8_t)1);   // Cente Align
  thermalPrinterSerial.write(0x1D); // GS
  thermalPrinterSerial.write('!'); // Text size
  thermalPrinterSerial.write((uint8_t)0x11); // Double height and width
  thermalPrinterSerial.write(0x1B); // ESC
  thermalPrinterSerial.write('E'); // Enable Bold
  thermalPrinterSerial.write((uint8_t)1);   // Turn bold ON
  thermalPrinterSerial.println("----------------"); // Long line
  thermalPrinterSerial.println("TIME TO LET GO");
  thermalPrinterSerial.println("----------------"); // Long line
  thermalPrinterSerial.write(0x1B); // ESC
  thermalPrinterSerial.write('E'); // Disable Bold
  thermalPrinterSerial.write((uint8_t)0);   // Turn bold OFF
  thermalPrinterSerial.write(0x1D); // GS
  thermalPrinterSerial.write('!'); // Reset text size
  thermalPrinterSerial.write((uint8_t)0x00); // Normal size
  // Time
  thermalPrinterSerial.println("[ " + time + " ]");
  thermalPrinterSerial.write(0x0A); // Line feed
  thermalPrinterSerial.write(0x0A); // Line feed
  // Content
  thermalPrinterSerial.write(0x1B); // ESC
  thermalPrinterSerial.write('a'); // Alignment
  thermalPrinterSerial.write((uint8_t)0);   // Left Align
  thermalPrinterSerial.println("Dear " + name + ",");
  thermalPrinterSerial.write(0x0A); // Line feed
  thermalPrinterSerial.print("I see you are feeling " + feelings + ", ");
  thermalPrinterSerial.println("and you wish to let go of: ");
  thermalPrinterSerial.println("\"" + description + "\"");
  thermalPrinterSerial.write(0x0A); // Line feed
  // Haiku
  thermalPrinterSerial.write(0x0A); // Line feed
  thermalPrinterSerial.println("Here is a haiku for you: ");
  thermalPrinterSerial.write(0x1B); // ESC
  thermalPrinterSerial.write('E'); // Enable Bold
  thermalPrinterSerial.write((uint8_t)1);   // Turn bold ON
  thermalPrinterSerial.write(0x0A); // Line feed
  thermalPrinterSerial.write(0x1D); // GS
  thermalPrinterSerial.write('!'); // Text size
  thermalPrinterSerial.write((uint8_t)0x10); // 1.5x height and width
  printHaikuText(haiku, 16);
  thermalPrinterSerial.write(0x1D); // GS
  thermalPrinterSerial.write('!'); // Reset text size
  thermalPrinterSerial.write((uint8_t)0x00); // Normal size
  thermalPrinterSerial.write(0x0A); // Line feed
  thermalPrinterSerial.write(0x1B); // ESC
  thermalPrinterSerial.write('E'); // Disable Bold
  thermalPrinterSerial.write((uint8_t)0);   // Turn bold OFF
  // Message
  thermalPrinterSerial.write(0x0A); // Line feed
  thermalPrinterSerial.println("Wishing you light and freedom,");
  thermalPrinterSerial.write(0x1B); // ESC
  thermalPrinterSerial.write('a'); // Alignment
  thermalPrinterSerial.write((uint8_t)2);   // Right Align
  thermalPrinterSerial.write(0x0A); // Line feed
  thermalPrinterSerial.println("-- Your Caring Companion");
  thermalPrinterSerial.write(0x0A); // Line feed
  thermalPrinterSerial.write(0x0A); 
  thermalPrinterSerial.write(0x0A); 
  thermalPrinterSerial.write(0x0A); 
  thermalPrinterSerial.write(0x0A); 
  thermalPrinterSerial.write(0x0A); 
  thermalPrinterSerial.write(0x0A); 
  thermalPrinterSerial.write(0x0A); 
  thermalPrinterSerial.write(0x0A); 
  // Send print status
  sendPrintStatusToDatabase("Completed");
}

void printHaikuText(String text, int maxCharsPerLine) {
  String line = "";
  while (text.length() > 0) {
    int spaceIndex = text.indexOf(' ');
    if (spaceIndex == -1) spaceIndex = text.length();
    String word = text.substring(0, spaceIndex);
    text = text.substring(spaceIndex + 1); 
    if (line.length() + word.length() + 1 > maxCharsPerLine) {
      thermalPrinterSerial.println(line);
      line = "";
    }
    if (line.length() > 0) line += " "; 
    line += word;
  }
  if (line.length() > 0) thermalPrinterSerial.println(line);
}

void sendPrintStatusToDatabase(String statusMessage) {
  String firebaseURL = "/printStatus.json?auth=" + String(firebaseAuth);
  String payload = "\"" + statusMessage + "\"";
  // Send PUT request
  Serial.println("Sending PUT request to: " + firebaseURL);
  http.put(firebaseURL, "application/json", payload); 
  http.stop(); 
}

void sendFireStateToDatabase(int fireState) {
  String firebaseURL = "/fireState.json?auth=" + String(firebaseAuth);
  String payload = String(fireState);
  Serial.println("Sending PUT request to: " + firebaseURL);
  http.put(firebaseURL, "application/json", payload); 
  http.stop(); 
}