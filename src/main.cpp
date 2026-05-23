#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <MPU6050.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <Adafruit_BME280.h>
#include <SPI.h>
#include <SD.h>

Adafruit_BME280 bme;
LiquidCrystal_I2C lcd(0x27, 16, 2); 
MPU6050 mpu;

#define BUTTON_PIN 3
#define BUTTON_LOG_PIN 2
#define GPS_RX_PIN 4
#define GPS_TX_PIN 5
#define SD_CS_PIN 10

TinyGPSPlus gps;
SoftwareSerial gpsSerial(GPS_RX_PIN, GPS_TX_PIN);

int currentPage = 1;
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 1000;

bool isLogging = false;
char currentLogFile[13];

void startLogging() {
  for (int i = 1; i < 1000; i++) {
    sprintf(currentLogFile, "TRK%03d.GPX", i);
    if (!SD.exists(currentLogFile)) break;
  }
  File f = SD.open(currentLogFile, FILE_WRITE);
  if (f) {
    f.println("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    f.println("<gpx version=\"1.1\" creator=\"CycloPath\">");
    f.println("<trk><name>Route</name><trkseg>");
    f.close();
    isLogging = true;
    lcd.clear(); 
    lcd.setCursor(0,0); lcd.print("Kayit: Basladi"); 
    lcd.setCursor(0,1); lcd.print(currentLogFile); 
    delay(1500);
  } else {
    lcd.clear(); 
    lcd.setCursor(0,0); lcd.print("SD KART HATASI"); 
    delay(1500);
  }
}

void stopLogging() {
  if (!isLogging) return;
  File f = SD.open(currentLogFile, FILE_WRITE);
  if (f) {
    f.println("</trkseg></trk>");
    f.println("</gpx>");
    f.close();
  }
  isLogging = false;
  lcd.clear(); 
  lcd.setCursor(0,0); lcd.print("Kayit: Bitti"); 
  lcd.setCursor(0,1); lcd.print(currentLogFile); 
  delay(1500);
}

void logGPSPoint() {
  if (!isLogging || !gps.location.isValid()) return;
  
  File f = SD.open(currentLogFile, FILE_WRITE);
  if (f) {
    f.print("<trkpt lat=\"");
    f.print(gps.location.lat(), 6);
    f.print("\" lon=\"");
    f.print(gps.location.lng(), 6);
    f.println("\">");
    
    if (gps.altitude.isValid()) {
      f.print("<ele>");
      f.print(gps.altitude.meters(), 1);
      f.println("</ele>");
    }
    
    f.println("</trkpt>");
    f.close();
  }
}

void setup() {
  Serial.begin(9600);
  Wire.begin(); 
  lcd.init();
  lcd.backlight();
  
  mpu.initialize();

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON_LOG_PIN, INPUT_PULLUP);
  
  if (!SD.begin(SD_CS_PIN)) {
    lcd.clear();
    lcd.print("SD KART BULUNAMADI");
    delay(2000); // Sadece uyari verir kodu kitlemez
  }

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
  bool logButtonState = digitalRead(BUTTON_LOG_PIN);

  // LOG BUTTON (Kayit Baslat / Bitir)
  if (logButtonState == LOW) {
    delay(50);
    if (digitalRead(BUTTON_LOG_PIN) == LOW) {
      if (isLogging) {
        stopLogging();
      } else {
        startLogging();
      }
      
      while(digitalRead(BUTTON_LOG_PIN) == LOW) {
        while (gpsSerial.available() > 0) gps.encode(gpsSerial.read());
      }
      lastUpdate = millis() - updateInterval; 
    }
  }

  // SAYFA BUTONU
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

    // Eger kayit aciksa her saniye (veya updateInterval neyse) GPX noktasini kaydet
    logGPSPoint();

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
      lcd.print("Alt: "); 
      if(isnan(alt)) lcd.print("--"); else lcd.print(alt, 0); 
      lcd.print("m ");
      
      lcd.print("T: "); 
      if(isnan(temp)) lcd.print("--"); else lcd.print(temp, 0); 
      lcd.print("C");

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
        lcd.print("Lat: Waiting ");
      }
      
      lcd.setCursor(0, 1);
      if (gps.location.isValid()) {
        lcd.print("Lon: ");
        lcd.print(gps.location.lng(), 5);
        lcd.print("   ");
      } else {
        lcd.print("Lon: Waiting ");
      }
    }
  }
}
