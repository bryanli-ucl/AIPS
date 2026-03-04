#include <Arduino.h>
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  delay(200);

  Wire.begin();
  Wire.setClock(100000); // 先用100k更稳，扫描出来再升到400k

  Serial.println("I2C scan start...");
}

void loop() {
  int found = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    uint8_t err = Wire.endTransmission();

    if (err == 0) {
      Serial.print("Found I2C device at 0x");
      if (addr < 16) Serial.print("0");
      Serial.println(addr, HEX);
      found++;
    }
  }
  if (!found) Serial.println("No I2C devices found.");

  Serial.println("Scan done.\n");
  delay(2000);
}