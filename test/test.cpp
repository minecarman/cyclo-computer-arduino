#include <Arduino.h>
#include <Wire.h>

void setup() {
  Wire.begin();
  Serial.begin(9600); 
  while (!Serial);   
  Serial.println("\nI2C Test");
}

void loop() {
  byte error, address;
  int nDevices;

  Serial.println("Scanning");
  nDevices = 0;

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("Device Address: 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      Serial.println("");

      nDevices++;
    }
    else if (error == 4) {
      Serial.print("Error Address: 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }

  if (nDevices == 0) {
    Serial.println("No Device.\n");
  } else {
    Serial.println("Scan Over.\n");
  }

  delay(3000);
}