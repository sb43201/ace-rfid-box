# ACE RFID Box User Manual

## ESP8266 + PN532 + 16x2 LCD Version

The ACE RFID Box reads, stores, creates, writes, and verifies Anycubic ACE-style filament RFID tag data. This branch is the experimental ESP8266 version for a WEMOS/Lolin D1 mini style board.

## Features

- Read compatible NTAG filament RFID tags.
- Create tags from built-in PLA, PETG, TPU, and ABS presets.
- Clone the currently loaded tag to another compatible writable tag.
- Save up to 20 tag records in ESP8266 EEPROM emulation.
- Load saved records and write them later.
- Verify that a written or cloned tag matches the current record.
- Print material, color, temperatures, diameter, length, UID, and raw page data in Serial Monitor.
- Detect Bambu/MIFARE-style tags as read-only/unsupported.
- Cancel tag wait screens with a long press on the encoder button.
- Turn off the ESP8266 WiFi radio at startup for lower battery use.

## Hardware

### Required Parts

- WEMOS/Lolin D1 mini ESP8266-compatible board
- 16x2 I2C LCD
- PN532 NFC/RFID module configured for I2C mode
- Rotary encoder with push button
- Read button
- Write button
- Active buzzer
- Onboard LED, plus optional external LED
- Compatible writable NFC tags

### Libraries

Install these libraries in Arduino IDE:

- `Adafruit PN532`
- `Adafruit BusIO`
- `LiquidCrystal_I2C`

`EEPROM.h` and `ESP8266WiFi.h` are included with the ESP8266 Arduino board package.

## Wiring

### I2C Bus

```text
ESP8266 D2 -> LCD SDA and PN532 SDA
ESP8266 D1 -> LCD SCL and PN532 SCL
3V3        -> LCD VCC and PN532 VCC if your modules support 3.3V
GND        -> LCD GND and PN532 GND
```

Set the PN532 board switch/jumpers to I2C mode.

### PN532

```text
PN532 IRQ   -> D5
PN532 RESET -> D0
```

### Controls

```text
Encoder CLK -> D6
Encoder DT  -> D7
Encoder SW  -> D3
Read button -> D4
Write button -> RX / GPIO3
Buzzer      -> D8
```

The sketch uses `INPUT_PULLUP`, so each button should connect the pin to GND when pressed.

Important: D3, D4, and D8 are ESP8266 boot strap pins. Avoid holding those buttons while powering or resetting the board.

## Using The Box

### Main Menu

Turn the encoder to move through:

- `Quick Preset`
- `Read Tag`
- `Clone Current`
- `Save Current`
- `Load Saved`
- `Write Preset`
- `Verify Current`
- `About`

Press the encoder button to select a menu item.

### Read A Tag

1. Choose `Read Tag` or press the read button.
2. Place a compatible tag on the PN532.
3. The LCD shows the material/color summary.
4. Serial Monitor prints detailed data, including temperatures and raw tag pages.

If a Bambu/MIFARE-style tag is detected, the LCD shows that it is read-only/unsupported and the UID is printed in Serial Monitor.

### Write A Preset

1. Choose `Quick Preset`.
2. Select the material group.
3. Select the filament color/preset.
4. Press the write button or choose `Write Preset`.
5. Place a compatible writable tag on the PN532.

### Clone Current

`Clone Current` writes the currently loaded tag data to another compatible writable tag. The current data can come from a successful read, a preset, or a loaded save slot.

### Save And Load

The ESP8266 version has 20 EEPROM-backed save slots.

- `Save Current` stores the current tag into the selected slot.
- `Load Saved` loads a slot back into current memory.

After loading a saved slot, use `Clone Current` or `Write Preset` to write it to a tag.

### Cancel A Tag Wait

When the screen says to place a tag, long-press the encoder button to cancel and return to the main menu.

## Serial Monitor

Use `115200` baud. The sketch prints:

- Startup messages
- Button and menu actions
- Tag UID
- Material and color
- Nozzle and bed temperature ranges
- Diameter and length
- Raw NFC page data
- Read/write/verify errors

## Limitations

- Bambu RFID tags can be detected, but this sketch cannot decode or write Bambu encrypted RFID data.
- ESP8266 EEPROM emulation is smaller than ESP32 flash storage, so this branch keeps 20 save slots.
- The write button uses RX/GPIO3. Disconnect anything using Serial RX if it interferes.
