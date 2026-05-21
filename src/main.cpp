#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <MPU6050.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme;
LiquidCrystal_I2C lcd(0x27, 16, 2); 
MPU6050 mpu;

#define BUTTON_PIN 3
#define GPS_RX_PIN 4
#define GPS_TX_PIN 5

TinyGPSPlus gps;
SoftwareSerial gpsSerial(GPS_RX_PIN, GPS_TX_PIN);

int currentPage = 1;
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 1000;

void setup() {
  Serial.begin(9600);
  Wire.begin(); 
  lcd.init();
  lcd.backlight();
  
  mpu.initialize();

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  gpsSerial.begin(9600);

  lcd.clear();
  lcd.print("CycloPath Ready");
  delay(2000);
}

void loop() {
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    Serial.write(c); // RAW GPS data test on serial monitor
    gps.encode(c);
  }

  bool currentButtonState = digitalRead(BUTTON_PIN);

  if (currentButtonState == LOW) { // Change page on button press
    delay(50);
    if (digitalRead(BUTTON_PIN) == LOW) {
      if (currentPage == 1) {
        currentPage = 2;
      } else {
        currentPage = 1;
      }
      lcd.clear(); 
      
      while(digitalRead(BUTTON_PIN) == LOW) {
        while (gpsSerial.available() > 0) {
          gps.encode(gpsSerial.read());
        }
      }
      lastUpdate = millis() - updateInterval; 
    }
  }

  if (millis() - lastUpdate >= updateInterval) {
    lastUpdate = millis();

    if (currentPage == 1) {
      float temp = bme.readTemperature();
      float alt = bme.readAltitude(1013.25);

      long sum_ax = 0, sum_az = 0;
      for (int i = 0; i < 20; i++) {
        int16_t ax, ay, az;
        mpu.getAcceleration(&ax, &ay, &az);
        sum_ax += ax;
        sum_az += az;
        delay(2);
      }
      float avg_ax = sum_ax / 20.0;
      float avg_az = sum_az / 20.0;

      float slope_pct = 0.0;
      if (abs(avg_az) > 100) {
        slope_pct = (avg_ax / avg_az) * 100.0;
      }
      
      if (slope_pct > 99.9) slope_pct = 99.9;
      if (slope_pct < -99.9) slope_pct = -99.9;

      lcd.setCursor(0, 0);
      lcd.print("Alt: "); lcd.print(alt, 0); lcd.print("m ");
      lcd.print("T: "); lcd.print(temp, 0); lcd.print("C");

      lcd.setCursor(0, 1);
      lcd.print("Slope: %"); 
      if(slope_pct >= 0) lcd.print("+");
      lcd.print(slope_pct, 1);
      lcd.print("    ");
    } 
    else if (currentPage == 2) {
      lcd.setCursor(0, 0);
      if (gps.location.isValid()) {
        lcd.print("Lat: ");
        lcd.print(gps.location.lat(), 5);
        lcd.print("   ");
      } else {
        lcd.print("Lat: Bekleniyor ");
      }
      
      lcd.setCursor(0, 1);
      if (gps.location.isValid()) {
        lcd.print("Lon: ");
        lcd.print(gps.location.lng(), 5);
        lcd.print("   ");
      } else {
        lcd.print("Lon: Bekleniyor ");
      }
    }
  }
}
