# ACE RFID Box - ESP8266 LCD

Author: Bin Sun

This branch targets the experimental WEMOS/Lolin D1 mini ESP8266 build using a 16x2 I2C LCD and PN532 in I2C mode.

## Required Arduino Libraries

- Adafruit PN532
- Adafruit BusIO
- LiquidCrystal_I2C

## Notes

The ESP8266 version uses EEPROM emulation for saved tags. It currently keeps 20 save slots because the ESP8266 EEPROM emulation size is limited compared with ESP32 flash `Preferences`.

For wiring, controls, workflows, and troubleshooting, see [USER_MANUAL.md](USER_MANUAL.md).
