#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_BME280.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_BME280 bme; 

void setup() {
  lcd.init();
  lcd.backlight();
  
  if (!bme.begin(0x76)) {
    lcd.print("BME Bulunamadi!");
    while (1);
  }
}

void loop() {
  float temp = bme.readTemperature();
  float alt = bme.readAltitude(1013.25);

  lcd.setCursor(0, 0);
  lcd.print("Sicaklik: ");
  lcd.print(temp);
  lcd.print(" C");

  lcd.setCursor(0, 1);
  lcd.print("Rakim: ");
  lcd.print(alt);
  lcd.print(" m");

  delay(2000);
}