# ACE RFID Box - ESP32 SH1106 OLED

Author: Bin Sun

This branch targets the WEMOS/Lolin ESP32 build using an I2C SH1106 OLED instead of the 16x2 I2C LCD.

## Required Arduino Libraries

- Adafruit PN532
- Adafruit BusIO
- Adafruit GFX Library
- Adafruit SH110X

## Display Wiring

```text
SH1106 VCC -> 3.3V
SH1106 GND -> GND
SH1106 SDA -> GPIO 21
SH1106 SCL -> GPIO 22
```

The OLED I2C address is set to `0x3C` in the sketch.

## Notes

The sketch keeps the existing menu and RFID behavior from the ESP32 LCD version. A small compatibility wrapper maps the old `lcd.clear()`, `lcd.setCursor()`, and `lcd.print()` calls onto the SH1106 OLED.

For wiring, controls, workflows, and troubleshooting, see [USER_MANUAL.md](USER_MANUAL.md).
