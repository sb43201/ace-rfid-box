# ACE RFID Box User Manual

## ESP32 + PN532 + SH1106 OLED Version

The ACE RFID Box reads, stores, creates, writes, and verifies RFID filament tag data. It is designed around an ESP32, a PN532 NFC module in I2C mode, and an SH1106 OLED module with rotary encoder and onboard buttons.

## Features

- Read filament RFID tags and display their information.
- Create tags from built-in PLA, PETG, TPU, and ABS presets.
- Clone the currently loaded tag to another compatible tag.
- Save up to 20 tag records in ESP32 flash memory.
- Load previously saved records and write them later.
- Verify that a written or cloned tag matches the current record.
- Show material, color, nozzle temperature, bed temperature, diameter, and filament length on the OLED.
- Detect Bambu/MIFARE-style tags and display their UID as read-only/unsupported.
- Dim the OLED after idle time and run the ESP32 at reduced CPU speed for battery use.
- Print operations and raw tag page data in Serial Monitor.

## Hardware

### Required Parts

- WEMOS/Lolin ESP32-compatible board
- SH1106 128x64 I2C OLED module with rotary encoder, `CONFIRM`, and `BACK` buttons
- PN532 NFC/RFID module configured for I2C mode
- Active buzzer
- Optional external LED with 220 ohm resistor
- Compatible NFC tags

### Libraries

Install these libraries in Arduino IDE:

- `Adafruit PN532`
- `Adafruit BusIO`
- `Adafruit GFX Library`
- `Adafruit SH110X`

`Preferences.h` is included with the ESP32 Arduino board package.

## Wiring

### SH1106 OLED / Control Module

| Module Pin | ESP32 Pin | Function |
| --- | --- | --- |
| `VCC` / `3V3-5V` | `3.3V` | Power |
| `GND` | `GND` | Ground |
| `SDA` / `OLED_SDA` | `GPIO 21` | OLED I2C data |
| `SCL` / `OLED_SCL` | `GPIO 22` | OLED I2C clock |
| `TRA` | `GPIO 32` | Encoder A / turn |
| `TRB` | `GPIO 33` | Encoder B / turn |
| `PSH` / `ENCODER_PUSH` | `GPIO 25` | Encoder button / select |
| `CON` / `CONFIRM` | `GPIO 26` | Read button |
| `BAK` / `BACK` | `GPIO 13` | Write button |

### PN532 Module

Set the PN532 module switch or jumpers to **I2C** mode.

| PN532 Pin | ESP32 Pin | Function |
| --- | --- | --- |
| `VCC` | Module-rated supply | Power |
| `GND` | `GND` | Ground |
| `SDA` | `GPIO 21` | I2C data |
| `SCL` | `GPIO 22` | I2C clock |
| `IRQ` | `GPIO 4` | Interrupt |
| `RSTO` / `RSTPDN` | `GPIO 16` | Reset |

The OLED and PN532 share `SDA` and `SCL`. This is normal for I2C devices.

### Buzzer And LED

| Device | ESP32 Pin | Notes |
| --- | --- | --- |
| Active buzzer `+` | `GPIO 14` | Buzzer `-` to `GND` |
| External LED anode | `GPIO 27` | Use a 220 ohm series resistor |
| External LED cathode | `GND` |  |

The onboard ESP32 LED is also used as a ready/success indication.

## First Start

1. Configure the PN532 module for I2C mode.
2. Wire the device while power is disconnected.
3. In Arduino IDE, open `ACE_RFID_Box_WEMOS_ESP32_SH1106.ino`.
4. Select the correct ESP32 board and port.
5. Upload the sketch.
6. Open Serial Monitor at `115200` baud.

At startup:

- The onboard LED turns on.
- The OLED shows the startup status.
- The PN532 is detected and initialized.
- A success double beep and LED flash indicates the device is ready.
- The ESP32 CPU is set to `80 MHz` to reduce power consumption.

If the PN532 is not detected, the OLED shows `PN532 ERROR` / `Check wiring` and the device stops at startup.

## Controls

| Control | Action |
| --- | --- |
| Rotate encoder | Scroll through menu items or choices |
| Press encoder button | Select the highlighted menu item or choice |
| `CONFIRM` onboard button | Immediately start `Read Tag` |
| `BACK` onboard button | Write/clone the current tag; if no current data exists, write the selected preset |
| Hold encoder while waiting for a tag | Cancel the wait and return to the main menu |
| Press any button on a filament information screen | Return to the main menu before the display timeout |

The main menu uses larger text. Submenus and detail screens use smaller text so more information fits on the OLED.

## Battery Saving

The SH1106 sketch includes basic battery-saving behavior:

- The ESP32 CPU is reduced to `80 MHz` during startup.
- Wi-Fi and Bluetooth are disabled during startup.
- The OLED dims after `30` seconds without control activity.
- Turning the encoder, pressing a button, or updating the screen wakes the OLED.

You can adjust the OLED idle dim time near the top of the sketch:

```cpp
const unsigned long oledDimAfterMs = 30000;
```

For example, use `10000` for 10 seconds or `60000` for 1 minute.

For bigger battery savings, use a physical power switch or regulator shutdown for the PN532 and OLED, because those modules still consume current even when the display is dimmed.

## Main Menu

### Quick Preset

Creates current tag data from built-in filament settings.

1. Select `Quick Preset`.
2. Rotate to choose material: `PLA`, `PETG`, `TPU`, or `ABS`.
3. Press the encoder to confirm the material.
4. Rotate to choose a color.
5. Press the encoder to make the preset current.

After selection, the OLED displays the preset information. Press any button to return sooner.

While choosing a color, pressing the `BACK` / Write button sets that preset and immediately begins writing it to a placed tag.

### Read Tag

Reads a physical tag into the current working memory.

1. Select `Read Tag`, or press the `CONFIRM` / Read button.
2. Place a tag on the PN532 reader.
3. Wait for the success beep and LED flashes.

The OLED shows:

- Material
- Color
- Nozzle temperature range
- Bed temperature range
- Diameter
- Length

Serial Monitor also prints this information and a raw data dump of tag pages `4` through `31`.

If a Bambu/MIFARE-style tag is detected, the OLED displays `Bambu/MIFARE`, the tag UID, and `Read-only detect`. The current sketch does not decode or write Bambu RFID data.

### Clone Current

Writes the current in-memory tag record to another compatible tag.

1. First create current data using `Read Tag`, `Quick Preset`, or `Load Saved`.
2. Select `Clone Current`.
3. Place the target tag on the PN532.
4. Wait for write and verification to complete.

Each written page is read back and compared byte-for-byte. A success indication means the verification passed.

### Save Current

Saves the current tag record in one of 20 flash-memory slots.

1. Load or create a current tag.
2. Select `Save Current`.
3. Rotate to choose slot `01` through `20`.
4. Press the encoder to save.

Saving to an occupied slot overwrites its previous contents.

### Load Saved

Loads one of the 20 stored records as the current tag.

1. Select `Load Saved`.
2. Rotate to choose a slot.
3. Press the encoder to load.

Empty slots are shown as `Empty` and cannot be loaded.

### Write Preset

Writes the currently selected preset to a tag.

The most recently selected `Quick Preset` is used. For clarity, select the desired preset using `Quick Preset` before choosing `Write Preset`.

### Verify Current

Checks whether a physical tag matches the current record.

1. Load, read, or create a current tag.
2. Select `Verify Current`.
3. Place the tag to check on the reader.

If every compared byte matches, the OLED displays the current filament information with a `Verify OK` status.

### About

Displays the device name and hardware version.

## Current Tag Concept

The device always works with one **current** record in memory. It can come from:

- A tag you just read
- A quick preset you selected
- A saved slot you loaded

`Clone Current`, `Save Current`, and `Verify Current` act on that record.

## Read / Write Indicators

| Indication | Meaning |
| --- | --- |
| Short beep when turning encoder | Selection changed |
| Click beep after button press/release | Button accepted |
| Double beep and LED double flash | Successful operation |
| Long low beep | Error or canceled NFC wait |
| Onboard LED on while ready | Device initialized |

## Stored And Displayed Filament Information

The sketch uses these tag fields:

| Information | Tag Data Location |
| --- | --- |
| Material/type | Pages `15-18` |
| Color bytes | Page `20` |
| Nozzle temperature min/max | Page `24` |
| Bed temperature min/max | Page `29` |
| Diameter and filament length | Page `30` |

The displayed color name is matched against the built-in preset color table. A tag with an unrecognized color code may display `Unknown`.

## Bambu / MIFARE Tag Detection

Bambu Lab filament RFID tags are different from the NTAG-style tags this writer is built around. They behave like MIFARE-style tags and do not expose the same simple page layout.

When the device detects a tag but the first NTAG page read fails, it treats the tag as likely unsupported and shows:

```text
Bambu/MIFARE
Tag detected
UID: ...
Read-only detect
```

This is detection only. It does not decode Bambu filament data, authenticate MIFARE sectors, clone a Bambu tag, or write a replacement Bambu tag.

## Serial Monitor

Open Arduino Serial Monitor at:

```text
115200 baud
```

Serial output includes:

- Startup initialization messages
- Menu choices and button actions
- Detected tag UID
- Read/write/verify status
- Material, color, temperature, diameter, and length
- Raw RFID tag page bytes

Serial Monitor is especially useful when verifying wiring or inspecting an unfamiliar tag.

## Troubleshooting

### OLED Is Blank

- Check `VCC`, `GND`, `SDA`, and `SCL`.
- Confirm the OLED is an SH1106 I2C display.
- The sketch uses address `0x3C`. If your module uses `0x3D`, update:

```cpp
#define OLED_ADDR 0x3D
```

### PN532 Error At Startup

- Ensure the PN532 is set to I2C mode.
- Confirm `SDA`, `SCL`, `IRQ`, `RSTO/RSTPDN`, power, and ground.
- Power-cycle after changing the PN532 mode switch.

### Buzzer Is Silent

- The sketch is currently configured for an active buzzer:

```cpp
#define BUZZER_IS_ACTIVE true
```

- Check buzzer polarity and wire `+` to `GPIO 14`, `-` to `GND`.
- If you use a passive buzzer, change the setting to `false`.

### Encoder Moves Too Fast

Increase the encoder interval value:

```cpp
const unsigned long encoderStepMs = 180;
```

For example, try `250`.

### Encoder Button Still Feels Sensitive

Increase the button debounce value:

```cpp
const unsigned long buttonDebounceMs = 90;
```

For example, try `130`.

### Tag Wait Takes Too Long

While the device is asking for a tag, hold the encoder button for about one second to cancel and return to the main menu.

### Bambu Tag Shows As Read-Only

This is expected. The sketch can identify that a Bambu/MIFARE-style tag is present and show its UID, but it cannot decode or write Bambu RFID data.

## Safety And Usage Notes

- Do not disconnect modules while the ESP32 is powered.
- Do not write to an original tag until you have first read and saved its contents.
- Test generated or cloned tags before depending on them in a printer workflow.
- Tag compatibility and exact filament metadata requirements may differ by printer or material system.
