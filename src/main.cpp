#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <MPU6050.h>

// Test için BMP180 kullanmak istiyorsaniz bu satiri aktif birakin, 
// orjinal BME280'e donmek icin yorum satiri yapin: //#define USE_BMP180
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

  lcd.clear();
  lcd.print("CycloPath Ready");
  delay(2000);
}

void loop() {
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

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Alt: "); lcd.print(alt, 0); lcd.print("m ");
  lcd.print("T: "); lcd.print(temp, 0); lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print("Slope: %"); 
  lcd.print(tan(angle * PI / 180) * 100, 1);

  delay(1000);
}