Time to Let Go
- An interactive experience designed to help individuals release lingering emotions and memories
- This project combines a web-based experience with a physical interaction, where users enter their thoughts, receive a generated haiku, print it, and then burn the paper—triggering a real-time visual response on the website.
- Physical setup includes an Arduino with WIFI, a thermal printer and a burning mechanism with flame sensor.

How It Works
- Open the physical box and access the companion website.
- Enter what you wish to let go of, based on a system-generated prompt.
- Review your input and receive a haiku generated in response to your input.
- Submit and print using the thermal printer.
- Take the paper as it emerges from the slot.
- Burn the paper with the provided lighter.
- Receive visual feedback from the website reflecting your action.
- You are free!!!

Technical Components
- Digital Part (Web App)
- Built with HTML, Javascript, CSS, used p5.js libary for interactive visuals.
- Data exchanged through wifi for wireless connection
- Firebase is used for real-time data exchange between the website and the Arduino.
- User input is sent to OpenAI API, which generates a personalized haiku.

Physical Part (Arduino & Printer System)
- Arduino Uno R4 Wifi 
- Thermal printer (RS-232 communication)
- Lighter
- Flame Sensor
- Power bank

Data Transmission Flow
- Website → Firebase
  User enters text and submits it.
  User input is stored in Firebase.
- Website → OpenAI API
  The website sends user input to OpenAI API.
  The API returns a haiku based on the input.
- OpenAI API → Firebase
  The haiku is stored in Firebase.
- Firebase → Arduino
  Arduino fetches the haiku and user input from Firebase.
- Arduino → Thermal Printer
  User input and haiku is formatted and printed.
- Arduino → Firebase
  Sends real-time updates on printing status and flame detection.
- Firebase → Website
  The website fetches updates and displays them to the user.
- Website
  Reacts dynamically, updating visuals when the paper is burned.
