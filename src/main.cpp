#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <MPU6050.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

// Comment out the following line to use the BME280 instead of the BMP180
#define USE_BMP180 

#ifdef USE_BMP180
#include <Adafruit_BMP085.h>
Adafruit_BMP085 bmp;
#else
#include <Adafruit_BME280.h>
Adafruit_BME280 bme;
#endif

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
  lcd.init();
  lcd.backlight();
  
#ifdef USE_BMP180
  if (!bmp.begin()) {
    lcd.print("BMP Error");
    while (1);
  }
#else
  if (!bme.begin(0x76)) {
    lcd.print("BME Error");
    while (1);
  }
#endif
  
  mpu.initialize();
  if (!mpu.testConnection()) {
    lcd.print("MPU Error");
    while (1);
  }

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  gpsSerial.begin(9600);

  lcd.clear();
  lcd.print("CycloPath Ready");
  delay(2000);
}

void loop() {
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  static bool lastButtonState = HIGH;
  bool currentButtonState = digitalRead(BUTTON_PIN);
  
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    currentPage = (currentPage == 1) ? 2 : 1;
    lcd.clear(); // Clear screen
    lastUpdate = millis() - updateInterval; // Force Update
    delay(50); // Debounce
  }
  lastButtonState = currentButtonState;

  if (millis() - lastUpdate >= updateInterval) {
    lastUpdate = millis();

    if (currentPage == 1) {
#ifdef USE_BMP180
      float temp = bmp.readTemperature();
      float alt = bmp.readAltitude(101325);
#else
      float temp = bme.readTemperature();
      float alt = bme.readAltitude(1013.25); 
#endif

      int16_t ax, ay, az;
      mpu.getAcceleration(&ax, &ay, &az);
      float angle = atan2(ax, az) * 180 / PI;

      lcd.setCursor(0, 0);
      lcd.print("Alt: "); lcd.print(alt, 0); lcd.print("m ");
      lcd.print("T: "); lcd.print(temp, 0); lcd.print("C");

      lcd.setCursor(0, 1);
      lcd.print("Slope: %"); 
      lcd.print(tan(angle * PI / 180) * 100, 1);
      lcd.print("    "); // Reset Screen
    } 
    else if (currentPage == 2) {
      lcd.setCursor(0, 0);
      lcd.print("Spd: ");
      if (gps.speed.isValid()) {
        lcd.print(gps.speed.kmph(), 1); 
        lcd.print(" km/h  ");
      } else {
        lcd.print("--.- km/h  ");
      }
      
      lcd.setCursor(0, 1);
      if (gps.location.isValid()) {
        lcd.print("GPS: Baglandi   "); 
      } else {
        lcd.print("GPS Yükleniyor.."); 
      }
    }
  }
}
