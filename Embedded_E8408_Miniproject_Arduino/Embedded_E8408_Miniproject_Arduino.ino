// Using library for the LCD-display: https://wiki.seeedstudio.com/Grove-LCD_RGB_Backlight/

#include <Wire.h>
#include "rgb_lcd.h"  // library for the LCD display
#include <stdint.h> // Library allowing usage of standard sized integer types

rgb_lcd lcd;

const int colorR = 0;
const int colorG = 0;
const int colorB = 255;
bool sync = 0;
unsigned int cal;
unsigned int MET = 6;
unsigned int hikeMinutes; // hike duration in minutes from LiLyGo
unsigned int hours; // hike hours calculated from the received minutes
unsigned int minutes; // hike leftover minutes from hour convert
uint8_t userWeight = 0; // user weight in kg, max value 255 to conserve memory
unsigned int stepCount; // steps count of the hike
unsigned int hikeDistance; // distance calculated by the LiLyGo watch

void setup() {
    // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);
    lcd.setRGB(colorR, colorG, colorB);

    // Print a welcomen message to the LCD.
    lcd.print("Hiking Assistant");

    // start serial for debugging
    Serial.begin(9600); 
    Serial1.begin(9600);
    delay(100);

    // start listening to the BLE dongle
    Serial.println("BT Module Initialized. Waiting for data...");
    delay(100);
}

void loop() {

    if (sync == 0){  // Checking if BT sync has been done already
    synchronize(); // Connect and download data from the watch
    }

    if (sync == 1){  // if BLE sync has been done, move on to data validation  andrefining
      cal = hikeMinutes * 60 * MET * 3.5 * userWeight / (200*60);  // use the received data to calculate burned calories
      ConvertData(); // convert hike duration from minutes to hours and minutes.
      DiplayUpdate(); // update display with the data
    }
}

void synchronize() {  
  lcd.setCursor(0, 1);
  lcd.print("Fetching data...");
  delay(100);
  
  // add bluetooth sync operations here. After succesfully receiving the data, set sync flag to true:-------------------------------------------!
  // data needed: time (seconds), weight (kg), steps

  while (sync == 0) {
    if (Serial1.available()) {
        Serial.println("Entered");
        String receivedData = Serial1.readStringUntil('\n'); // Read BLE data line by line
        if (parseData(receivedData)) { // Check if received data is valid
            Serial1.println("ACK"); // Send acknowledgment back to the sender
            Serial.println("ACK");
            sync = 1; // Record that sync has been done
        }
    }
  }
  // Print received data to Serial Monitor for debugging
  Serial.print("Weight: "); Serial.print(userWeight); Serial.print(" kg, ");
  Serial.print("Duration: "); Serial.print(hikeMinutes); Serial.print(" min, ");
  Serial.print("Steps: "); Serial.print(stepCount); Serial.print(", ");
  Serial.print("Distance: "); Serial.print(hikeDistance); Serial.println(" m");

  // Show to the user that data has been received
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("DONE!");
  delay (1000);

  return;
}

bool parseData(String data) {
    // Expected format: "W:70,T:120,S:5000,D:2000" (Weight in kg, Duration in min, Steps count)
    int wIndex = data.indexOf("W:");
    int tIndex = data.indexOf(",T:");
    int sIndex = data.indexOf(",S:");
    int dIndex = data.indexOf(",D:");
    
    if (wIndex != -1 && dIndex != -1 && sIndex != -1 && tIndex != -1) {
        uint8_t tempWeight = (uint8_t)data.substring(wIndex + 2, tIndex).toInt();
        unsigned int tempDuration = data.substring(tIndex + 3, sIndex).toInt();
        unsigned int tempSteps = data.substring(sIndex + 3, dIndex).toInt();
        unsigned int tempDistance = data.substring(dIndex + 3).toInt();
        
        // Validate values before storing
        if (tempWeight > 0 && tempWeight < 255 && tempDuration > 0 && tempSteps > 0) {
            userWeight = tempWeight;
            hikeMinutes = tempDuration;
            stepCount = tempSteps;
            hikeDistance = tempDistance;
            return true; // Data is valid and stored
        }
    }
    return false; // Invalid data
}

void ConvertData(){
  hours = hikeMinutes/60; // how many hours was the hike (rounded down)
  minutes = (hikeMinutes % 60); // leftover minutes
}

void DiplayUpdate(){

    lcd.clear(); // clear the screen
    // present the duration and distance data
    lcd.setCursor(0, 0);
    lcd.print("Time & Distance");
    lcd.setCursor(0, 1);
    lcd.print(hours); // in format h:mm?
    lcd.setCursor(1, 1);
    lcd.print(":"); 
    lcd.setCursor(2, 1);
    lcd.print(minutes);
    lcd.print(" h:m");
    lcd.setCursor(8, 1);
    lcd.print(hikeDistance, 4); // tsekkaa tuleeko tämä oikein neljän merkin tarkkuudella? tyyliin XX.XX tai X.XXX ------------------------------------------------!
    lcd.setCursor(13, 1);
    lcd.print("km");

    delay (5000); // wait for user to read the data (could be done with a button too)
    lcd.clear();

    // continue to present the rest of the data
    lcd.setCursor(0, 0);
    lcd.print("Calories burned");
    lcd.setCursor(0, 1);
    lcd.print(cal);
    lcd.setCursor(6, 1);
    lcd.print("kcal");
    delay (5000); // wait for user to read the data (could be done with a button too)
    lcd.clear();

    // continue to present the rest of the data
    lcd.setCursor(0, 0);
    lcd.print("Total steps");
    lcd.setCursor(0, 1);
    lcd.print(stepCount);
    delay (5000); // wait for user to read the data (could be done with a button too)
    lcd.clear();

    delay (100);
}