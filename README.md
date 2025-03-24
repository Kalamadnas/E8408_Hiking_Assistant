# E8408_Hiking_Tour_Assistant

Hiking Tour Assistant
Overview
Miniproject from the ELEC-E8408 course, Aalto University.
Hiking Tour Assistant is a wearable Arduino-based system designed to track hiking sessions. A smart wristband collects real-time activity data and transmits step and distance data via Bluetooth Low Energy (BLE) to an Arduino, ensuring seamless synchronization.



System Components
1.  Smartwatch 
* Tracks user activity (steps, distance).
* Stores session data until synced with the Arduino.
* Uses Bluetooth for communication.
* Sends data in a structured format for processing.
2. Arduino 
* Bluetooth connection with the wristband.
* Receives and processes hiking session data.
* Sends acknowledgment (r) upon successful data reception.
Supported hardware
Arduino Mega
LCD Screen (16x2, I2C)
LilyGo Smartwatch (ESP32) 
Supported Software 
Arduino IDE 2.3.4 

Code written in C/C++ 

  Libraries used: 
- TTGO_TWatch_Library V1.4.3
- esp32 2.0.5
- Grove-LCD RGB Backlight
Setup & Installation
Bluetooth enabled wristband
Arduino board with BLE module
Installation
The software is uploaded to the watch and Arduino Mega via the Arduino IDE upload-function while the devices are connected to a computer with USB-cables. 
More detailed installation can be found in user manual.
Contact
Markus Mattson markus.e.mattsson@aalto.fi
Teemu Rauha teemu.rauha@aalto.fi
Vilma Väisänen vilma.e.vaisanen@aalto.fi
