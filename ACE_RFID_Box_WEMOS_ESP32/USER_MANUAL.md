# ACE RFID Box User Manual

## ESP32 + PN532 + 16x2 LCD Version

The ACE RFID Box reads, stores, creates, writes, and verifies RFID filament tag data. This version uses an ESP32, a PN532 NFC module in I2C mode, a 16x2 I2C LCD, a rotary encoder, read/write buttons, a buzzer, and an optional external LED.

## Features

- Read filament RFID tags and display their basic information.
- Create tags from built-in PLA, PETG, TPU, and ABS presets.
- Clone the current tag record to another compatible tag.
- Save up to 20 tag records in ESP32 flash memory.
- Load saved records and write them later.
- Verify that a written or cloned tag matches the current record.
- Detect Bambu/MIFARE-style tags and show that they are read-only/unsupported.
- Print operations, tag UID, filament details, and raw tag page data in Serial Monitor.
- Disable Wi-Fi and Bluetooth at startup for lower battery use.

## Hardware

### Required Parts

- WEMOS/Lolin ESP32-compatible board
- 16x2 I2C LCD
- PN532 NFC/RFID module configured for I2C mode
- Rotary encoder with push button
- Read and write buttons
- Active buzzer
- Optional external LED with 220 ohm resistor
- Compatible NFC tags

### Libraries

Install these libraries in Arduino IDE:

- `LiquidCrystal_I2C`
- `Adafruit PN532`
- `Adafruit BusIO`

`Preferences.h`, `WiFi.h`, and `esp_bt.h` are included with the ESP32 Arduino board package.

## Wiring

### 16x2 I2C LCD

| LCD Pin | ESP32 Pin | Function |
| --- | --- | --- |
| `VCC` | `5V` or module-rated supply | Power |
| `GND` | `GND` | Ground |
| `SDA` | `GPIO 21` | I2C data |
| `SCL` | `GPIO 22` | I2C clock |

The LCD address is set to `0x27` in the sketch.

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

The LCD and PN532 share `SDA` and `SCL`. This is normal for I2C devices.

### Rotary Encoder And Buttons

| Control | ESP32 Pin | Function |
| --- | --- | --- |
| Encoder `CLK` | `GPIO 32` | Encoder turn A |
| Encoder `DT` | `GPIO 33` | Encoder turn B |
| Encoder `SW` | `GPIO 25` | Encoder select |
| Read button | `GPIO 26` | Start `Read Tag` |
| Write button | `GPIO 13` | Clone/write current tag |

Buttons are configured with `INPUT_PULLUP`, so the normal wiring is one side to the GPIO pin and the other side to `GND`.

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
3. In Arduino IDE, open `ACE_RFID_Box_WEMOS_ESP32.ino`.
4. Select the correct ESP32 board and port.
5. Upload the sketch.
6. Open Serial Monitor at `115200` baud.

At startup:

- Wi-Fi and Bluetooth are disabled to reduce battery use.
- The onboard LED turns on.
- The LCD shows startup status.
- The PN532 is detected and initialized.
- A success double beep and LED flash indicates the device is ready.

If the PN532 is not detected, the LCD shows `PN532 ERROR` / `Check wiring` and the device stops at startup.

## Controls

| Control | Action |
| --- | --- |
| Rotate encoder | Scroll through menu items or choices |
| Press encoder button | Select the highlighted menu item or choice |
| Read button | Immediately start `Read Tag` |
| Write button | Clone/write the current tag; if no current data exists, write the selected preset |
| Hold encoder while waiting for a tag | Cancel the wait and return to the main menu |

## Main Menu

### Quick Preset

Creates current tag data from built-in filament settings.

1. Select `Quick Preset`.
2. Rotate to choose material: `PLA`, `PETG`, `TPU`, or `ABS`.
3. Press the encoder to confirm the material.
4. Rotate to choose a color.
5. Press the encoder to make the preset current.

While choosing a color, pressing the Write button sets that preset and immediately begins writing it to a placed tag.

### Read Tag

Reads a physical tag into the current working memory.

1. Select `Read Tag`, or press the Read button.
2. Place a tag on the PN532 reader.
3. Wait for the success beep and LED flashes.

The LCD shows material and color. Serial Monitor prints material, color, nozzle temperature, bed temperature, diameter, length, and a raw page dump of tag pages `4` through `31`.

If a Bambu/MIFARE-style tag is detected, the LCD shows that it is read-only/unsupported and the UID is printed in Serial Monitor.

### Clone Current

Writes the current in-memory tag record to another compatible tag.

1. First create current data using `Read Tag`, `Quick Preset`, or `Load Saved`.
2. Select `Clone Current`, or press the Write button.
3. Place the target tag on the PN532.
4. Wait for write and verification to complete.

Each written page is read back and compared byte-for-byte.

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

Empty slots are shown as empty and cannot be loaded.

### Write Preset

Writes the currently selected preset to a tag.

The most recently selected `Quick Preset` is used. For clarity, select the desired preset using `Quick Preset` before choosing `Write Preset`.

### Verify Current

Checks whether a physical tag matches the current record.

1. Load, read, or create a current tag.
2. Select `Verify Current`.
3. Place the tag to check on the reader.

If every compared byte matches, the LCD displays `VERIFY PASS`.

### About

Displays the device name and hardware version.

## Current Tag Concept

The device always works with one **current** record in memory. It can come from:

- A tag you just read
- A quick preset you selected
- A saved slot you loaded

`Clone Current`, `Save Current`, and `Verify Current` act on that record.

## Bambu / MIFARE Tag Detection

Bambu Lab filament RFID tags are different from the NTAG-style tags this writer is built around. They behave like MIFARE-style tags and do not expose the same simple page layout.

When the device detects a tag but the first NTAG page read fails, it treats the tag as likely unsupported and shows:

```text
Bambu/MIFARE
Read-only tag
```

Then it shows:

```text
UID in Serial
Not writable
```

This is detection only. It does not decode Bambu filament data, authenticate MIFARE sectors, clone a Bambu tag, or write a replacement Bambu tag.

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
- Bambu/MIFARE read-only detection details

## Battery Saving

The LCD sketch disables Wi-Fi and Bluetooth during startup because this project does not need radio features.

For bigger battery savings, use a physical power switch or regulator shutdown for the PN532 and LCD, because those modules still consume current while powered.

## Troubleshooting

### LCD Is Blank

- Check `VCC`, `GND`, `SDA`, and `SCL`.
- Confirm the I2C address. The sketch uses `0x27`; some LCD backpacks use `0x3F`.
- Adjust the contrast potentiometer on the LCD backpack.

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

### Tag Wait Takes Too Long

While the device is asking for a tag, hold the encoder button for about one second to cancel and return to the main menu.

### Bambu Tag Shows As Read-Only

This is expected. The sketch can identify that a Bambu/MIFARE-style tag is present and show its UID in Serial Monitor, but it cannot decode or write Bambu RFID data.

## Safety And Usage Notes

- Do not disconnect modules while the ESP32 is powered.
- Do not write to an original tag until you have first read and saved its contents.
- Test generated or cloned tags before depending on them in a printer workflow.
- Tag compatibility and exact filament metadata requirements may differ by printer or material system.
