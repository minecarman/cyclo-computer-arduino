#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_BME280.h>
#include <MPU6050.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); 
Adafruit_BME280 bme;
MPU6050 mpu;

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  
  if (!bme.begin(0x76)) {
    lcd.print("BME Hatasi!");
    while (1);
  }
  
  mpu.initialize();
  if (!mpu.testConnection()) {
    lcd.print("MPU Hatasi!");
    while (1);
  }

  lcd.clear();
  lcd.print("CycloPath Hazir!");
  delay(2000);
}

void loop() {
  float temp = bme.readTemperature();
  float alt = bme.readAltitude(1013.25); 

  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);
  float angle = atan2(ax, az) * 180 / PI;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Alt: "); lcd.print(alt, 0); lcd.print("m ");
  lcd.print("T: "); lcd.print(temp, 0); lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print("Egim: %"); 
  lcd.print(tan(angle * PI / 180) * 100, 1);

  delay(1000);
}