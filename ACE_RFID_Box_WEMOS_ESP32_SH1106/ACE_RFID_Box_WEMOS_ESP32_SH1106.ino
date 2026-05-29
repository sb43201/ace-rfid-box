#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_PN532.h>
#include <Preferences.h>

// ===================== WEMOS MINI ESP32 PIN SETTINGS =====================
#define I2C_SDA       21
#define I2C_SCL       22

#define PN532_IRQ     4
#define PN532_RESET   16

#define ENC_CLK       32
#define ENC_DT        33
#define ENC_SW        25

#define READ_BUTTON   26
#define WRITE_BUTTON  13

#define BUZZER_PIN    14
#define LED_PIN       27
#define BUZZER_IS_ACTIVE true

#define OLED_ADDR      0x3C
#define OLED_WIDTH     128
#define OLED_HEIGHT    64
#define OLED_RESET     -1

const unsigned long infoDisplayMs = 8000;
const unsigned long shortStatusMs = 2500;

#ifndef LED_BUILTIN
#define LED_BUILTIN   2
#endif

// ===================== OBJECTS =====================
Adafruit_SH1106G display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET, &Wire);
Preferences prefs;

class OledLcdCompat {
public:
  void begin() {
    display.begin(OLED_ADDR, true);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 0);
    display.display();
  }

  void init() {
    begin();
  }

  void backlight() {
  }

  void clear() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 0);
    display.display();
  }

  void setCursor(int col, int row) {
    display.setCursor(col * 6, row * 12);
  }

  void print(const char* text) {
    display.print(text);
    display.display();
  }

  void print(char c) {
    display.print(c);
    display.display();
  }

  void print(const String &text) {
    display.print(text);
    display.display();
  }

  void print(int value) {
    display.print(value);
    display.display();
  }

  void println(const char* text) {
    display.println(text);
    display.display();
  }
};

OledLcdCompat lcd;

// ===================== TAG DATA =====================
struct TagData {
  uint8_t pages[32][4];
  bool valid;
  char name[17];
  char material[17];
  char colorName[17];
  int nozzleMin;
  int nozzleMax;
  int bedMin;
  int bedMax;
  int diameter;
  int lengthMeter;
};

TagData currentTag;

// ===================== PRESET DATA =====================
struct FilamentPreset {
  const char* name;
  const char* material;
  const char* colorName;
  const char* colorHex;
  const char* sku;
  int nozzleMin;
  int nozzleMax;
  int bedMin;
  int bedMax;
  int diameter;
  int lengthMeter;
};

FilamentPreset presets[] = {
  {"PLA Black", "PLA", "Black", "212721FF", "ACPLA-BLK", 190, 230, 50, 60, 175, 330},
  {"PLA White", "PLA", "White", "EFF0F1FF", "AHPLWH-101", 190, 230, 50, 60, 175, 330},
  {"PLA Grey", "PLA", "Grey", "B1B3B3FF", "ACPLA-GRY", 190, 230, 50, 60, 175, 330},
  {"PLA Red", "PLA", "Red", "CE3845FF", "ACPLA-RED", 190, 230, 50, 60, 175, 330},
  {"PLA Yellow", "PLA", "Yellow", "F3E500FF", "ACPLA-YEL", 190, 230, 50, 60, 175, 330},
  {"PLA Blue", "PLA", "Blue", "003594FF", "ACPLA-BLU", 190, 230, 50, 60, 175, 330},
  {"PLA Green", "PLA", "Green", "009639FF", "ACPLA-GRN", 190, 230, 50, 60, 175, 330},
  {"PLA Purple", "PLA", "Purple", "6A6DCDFF", "ACPLA-PUR", 190, 230, 50, 60, 175, 330},
  {"PLA Orange", "PLA", "Orange", "FF7F32FF", "ACPLA-ORG", 190, 230, 50, 60, 175, 330},
  {"PLA Pink", "PLA", "Pink", "FF8DA1FF", "ACPLA-PNK", 190, 230, 50, 60, 175, 330},
  {"PLA Green Flash", "PLA", "Green Flash", "75CB5DFF", "ACPLA-GFL", 190, 230, 50, 60, 175, 330},
  {"PLA Texture Grey", "PLA", "Texture Grey", "75787BFF", "ACPLA-TGY", 190, 230, 50, 60, 175, 330},
  {"PLA Beige", "PLA", "Beige", "D4B996FF", "ACPLA-BGE", 190, 230, 50, 60, 175, 330},
  {"PLA Bronze", "PLA", "Bronze", "7C4D3AFF", "ACPLA-BRZ", 190, 230, 50, 60, 175, 330},
  {"PLA Brown", "PLA", "Brown", "927968FF", "ACPLA-BRN", 190, 230, 50, 60, 175, 330},
  {"PLA Dark Brown", "PLA", "Dark Brown", "975E3EFF", "ACPLA-DBR", 190, 230, 50, 60, 175, 330},
  {"PLA Texture Silver", "PLA", "Texture Silver", "8A8D8FFF", "ACPLA-TSV", 190, 230, 50, 60, 175, 330},
  {"PLA Cyan", "PLA", "Cyan", "23A3C7FF", "ACPLA-CYN", 190, 230, 50, 60, 175, 330},
  {"PLA Magenta", "PLA", "Magenta", "CF4F80FF", "ACPLA-MAG", 190, 230, 50, 60, 175, 330},
  {"PLA Clear", "PLA", "Clear", "FFFFFFFF", "ACPLA-CLR", 190, 230, 50, 60, 175, 330},

  {"PETG Black", "PETG", "Black", "212721FF", "ACPETG-BLK", 230, 250, 60, 70, 175, 330},
  {"PETG White", "PETG", "White", "EFF0F1FF", "AHPETGWH-101", 230, 250, 60, 70, 175, 330},
  {"PETG Grey", "PETG", "Grey", "97999BFF", "ACPETG-GRY", 230, 250, 60, 70, 175, 330},
  {"PETG Red", "PETG", "Red", "C8102EFF", "ACPETG-RED", 230, 250, 60, 70, 175, 330},
  {"PETG Yellow", "PETG", "Yellow", "F3E500FF", "ACPETG-YEL", 230, 250, 60, 70, 175, 330},
  {"PETG Blue", "PETG", "Blue", "003594FF", "ACPETG-BLU", 230, 250, 60, 70, 175, 330},
  {"PETG Green", "PETG", "Green", "009639FF", "ACPETG-GRN", 230, 250, 60, 70, 175, 330},
  {"PETG Purple", "PETG", "Purple", "6A6DCDFF", "ACPETG-PUR", 230, 250, 60, 70, 175, 330},
  {"PETG Orange", "PETG", "Orange", "FF7F32FF", "ACPETG-ORG", 230, 250, 60, 70, 175, 330},
  {"PETG Pink", "PETG", "Pink", "FB637EFF", "ACPETG-PNK", 230, 250, 60, 70, 175, 330},
  {"PETG Texture Grey", "PETG", "Texture Grey", "75787BFF", "ACPETG-TGY", 230, 250, 60, 70, 175, 330},
  {"PETG Texture Silver", "PETG", "Texture Silver", "8A8D8FFF", "ACPETG-TSV", 230, 250, 60, 70, 175, 330},
  {"PETG Dark Grey", "PETG", "Dark Grey", "7E868AFF", "ACPETG-DGY", 230, 250, 60, 70, 175, 330},
  {"PETG Brown", "PETG", "Brown", "927968FF", "ACPETG-BRN", 230, 250, 60, 70, 175, 330},
  {"PETG Beige", "PETG", "Beige", "D4B996FF", "ACPETG-BGE", 230, 250, 60, 70, 175, 330},
  {"PETG Peanut Brown", "PETG", "Peanut Brown", "A9754FFF", "ACPETG-PBN", 230, 250, 60, 70, 175, 330},
  {"PETG Cream", "PETG", "Cream", "F9DFB9FF", "ACPETG-CRM", 230, 250, 60, 70, 175, 330},
  {"PETG Lake Blue", "PETG", "Lake Blue", "0084D4FF", "ACPETG-LBU", 230, 250, 60, 70, 175, 330},
  {"PETG Forest Green", "PETG", "Forest Green", "43523BFF", "ACPETG-FGN", 230, 250, 60, 70, 175, 330},
  {"PETG Lime Green", "PETG", "Lime Green", "78D64BFF", "ACPETG-LGN", 230, 250, 60, 70, 175, 330},
  {"PETG Clear", "PETG", "Clear", "FFFFFFFF", "ACPETG-CLR", 230, 250, 60, 70, 175, 330},

  {"TPU Black", "TPU", "Black", "212721FF", "ACTPU-BLK", 195, 230, 50, 60, 175, 330},
  {"TPU Grey", "TPU", "Grey", "63666AFF", "ACTPU-GRY", 195, 230, 50, 60, 175, 330},
  {"TPU Milky White", "TPU", "Milky White", "E9E9E7FF", "ACTPU-MWH", 195, 230, 50, 60, 175, 330},
  {"TPU Red", "TPU", "Red", "D22630FF", "ACTPU-RED", 195, 230, 50, 60, 175, 330},
  {"TPU Orange", "TPU", "Orange", "FF6A13FF", "ACTPU-ORG", 195, 230, 50, 60, 175, 330},
  {"TPU Purple", "TPU", "Purple", "A438A8FF", "ACTPU-PUR", 195, 230, 50, 60, 175, 330},
  {"TPU Blue", "TPU", "Blue", "005EB8FF", "ACTPU-BLU", 195, 230, 50, 60, 175, 330},
  {"TPU Green", "TPU", "Clear Green", "79C000FF", "ACTPU-GRN", 195, 230, 50, 60, 175, 330},
  {"TPU Clear", "TPU", "Clear", "FFFFFFFF", "ACTPU-CLR", 195, 230, 50, 60, 175, 330},

  {"ABS Black", "ABS", "Black", "212721FF", "ACABS-BLK", 240, 260, 90, 100, 175, 330},
  {"ABS White", "ABS", "White", "ECECE7FF", "ACABS-WHT", 240, 260, 90, 100, 175, 330},
  {"ABS Grey", "ABS", "Grey", "A7A8AAFF", "ACABS-GRY", 240, 260, 90, 100, 175, 330},
  {"ABS Red", "ABS", "Red", "D6001CFF", "ACABS-RED", 240, 260, 90, 100, 175, 330},
  {"ABS Orange", "ABS", "Orange", "FF671FFF", "ACABS-ORG", 240, 260, 90, 100, 175, 330},
  {"ABS Yellow", "ABS", "Yellow", "FFE900FF", "ACABS-YEL", 240, 260, 90, 100, 175, 330},
  {"ABS Green", "ABS", "Green", "00B140FF", "ACABS-GRN", 240, 260, 90, 100, 175, 330},
  {"ABS Blue", "ABS", "Blue", "00239CFF", "ACABS-BLU", 240, 260, 90, 100, 175, 330}
};

const int presetCount = sizeof(presets) / sizeof(presets[0]);

// ===================== MENU =====================
const char* menuItems[] = {
  "Quick Preset",
  "Read Tag",
  "Clone Current",
  "Save Current",
  "Load Saved",
  "Write Preset",
  "Verify Current",
  "About"
};

const int menuCount = sizeof(menuItems) / sizeof(menuItems[0]);

const char* materialFilters[] = {"PLA", "PETG", "TPU", "ABS"};
const int materialFilterCount = sizeof(materialFilters) / sizeof(materialFilters[0]);

int menuIndex = 0;
int selectedPreset = 0;
int selectedMaterial = 0;
const int saveSlotCount = 20;
int selectedSaveSlot = 0;
bool tagWaitCanceled = false;

// ===================== ENCODER STATE =====================
int lastCLK = HIGH;
unsigned long lastMove = 0;
// Raise this value if one physical detent still advances more than one item.
const unsigned long encoderStepMs = 180;

// ===================== BUTTON STATE =====================
const unsigned long buttonDebounceMs = 90;

struct ButtonState {
  int pin;
  bool stableState;
  bool lastReading;
  bool clickReady;
  unsigned long lastChange;
};

ButtonState encoderButton = {ENC_SW, HIGH, HIGH, false, 0};
ButtonState readButton = {READ_BUTTON, HIGH, HIGH, false, 0};
ButtonState writeButton = {WRITE_BUTTON, HIGH, HIGH, false, 0};

// ===================== FUNCTION DECLARATIONS =====================
void handleEncoder();
int readEncoderStep();
void handleButtons();
bool buttonPressed(int pin);
ButtonState* getButtonState(int pin);
void updateButton(ButtonState &button);
void lcdPrintTrimmed(const char* text);
void lcdPrintTrimmedWidth(const char* text, int width);
void lcdPrintTagName(const char* label, const char* name);
void lcdPrintFilamentInfo(const char* action, TagData &tag);
void oledShowTwoLineMenu(const char* title, const char* value);
void oledShowCompactPicker(const char* title, const char* value);
void holdInfoOrDismiss();
String uidToString(uint8_t* uid, uint8_t uidLength);
void showUnsupportedMifareTag(uint8_t* uid, uint8_t uidLength);
void showMenu();
void runMenuAction();
void selectQuickPreset();
void selectMaterialFilter();
bool presetMatchesMaterial(int presetIndex, int materialIndex);
int firstPresetForMaterial(int materialIndex);
int nextPresetForMaterial(int currentIndex, int materialIndex, int dir);
void makePresetCurrent(FilamentPreset p);
void readTagToCurrent();
void cloneCurrentToTag();
void saveCurrentTag();
void loadSavedTag();
int selectSaveSlot(const char* title);
String slotKey(const char* prefix, int slot);
void writeSelectedPreset();
void verifyCurrentTag();
bool waitForTag(uint8_t* uid, uint8_t* uidLength, unsigned long timeoutMs);
bool writeTagData(TagData tag);
void buildPresetTag(FilamentPreset p, TagData &tag);
void writeStringToPages(TagData &tag, int startPage, const char* text, int maxBytes);
void makeTwoIntPage(int a, int b, uint8_t output[4]);
void decodeTagName(TagData &tag);
void decodeTagColor(TagData &tag);
void decodeTagSettings(TagData &tag);
int readTwoByteInt(uint8_t lowByte, uint8_t highByte);
void dumpCurrentToSerial();
void hexToByte(const char* hexString, uint8_t* byteArray, int byteCount);
void reverseArray(uint8_t* array, int len);
void showAbout();
void playTone(int frequency, int durationMs);
void buzzerOff();
void beepTurn();
void beepClick();
void beepSuccess();
void beepFail();
void showError(const char* msg);

// ===================== SETUP =====================
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("ACE RFID Box booting");

  pinMode(ENC_CLK, INPUT_PULLUP);
  pinMode(ENC_DT, INPUT_PULLUP);
  pinMode(ENC_SW, INPUT_PULLUP);
  pinMode(READ_BUTTON, INPUT_PULLUP);
  pinMode(WRITE_BUTTON, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(LED_BUILTIN, HIGH);
  buzzerOff();

  currentTag.valid = false;
  strcpy(currentTag.name, "Empty");
  strcpy(currentTag.material, "Unknown");
  strcpy(currentTag.colorName, "Unknown");
  currentTag.nozzleMin = 0;
  currentTag.nozzleMax = 0;
  currentTag.bedMin = 0;
  currentTag.bedMax = 0;
  currentTag.diameter = 0;
  currentTag.lengthMeter = 0;

  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.println("I2C started");

  lcd.begin();
  lcd.backlight();
  Serial.println("LCD started");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Anycubic RFID");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");

  nfc.begin();
  Serial.println("PN532 begin");

  uint32_t versiondata = nfc.getFirmwareVersion();

  if (!versiondata) {
    Serial.println("ERROR: PN532 not found");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("PN532 ERROR");
    lcd.setCursor(0, 1);
    lcd.print("Check wiring");
    beepFail();
    while (1);
  }

  nfc.SAMConfig();
  Serial.print("PN532 firmware: 0x");
  Serial.println(versiondata, HEX);

  prefs.begin("ace-rfid", false);
  Serial.println("Preferences opened");

  beepSuccess();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PN532 Ready");
  lcd.setCursor(0, 1);
  lcd.print("SH1106 OLED");
  delay(1500);

  showMenu();
}

// ===================== LOOP =====================
void loop() {
  handleEncoder();
  handleButtons();
}

// ===================== ENCODER =====================
void handleEncoder() {
  int dir = readEncoderStep();
  if (dir == 0) return;

  menuIndex += dir;

  if (menuIndex < 0) menuIndex = menuCount - 1;
  if (menuIndex >= menuCount) menuIndex = 0;

  Serial.print("Menu selected: ");
  Serial.println(menuItems[menuIndex]);
  beepTurn();
  showMenu();
}

int readEncoderStep() {
  int currentCLK = digitalRead(ENC_CLK);
  int dir = 0;

  if (currentCLK != lastCLK &&
      currentCLK == LOW &&
      millis() - lastMove >= encoderStepMs) {
    dir = digitalRead(ENC_DT) != currentCLK ? 1 : -1;
    lastMove = millis();
  }

  lastCLK = currentCLK;
  return dir;
}

// ===================== BUTTONS =====================
void handleButtons() {
  if (buttonPressed(ENC_SW)) {
    Serial.println("Encoder button pressed");
    runMenuAction();
  }

  if (buttonPressed(READ_BUTTON)) {
    Serial.println("Read button pressed");
    readTagToCurrent();
  }

  if (buttonPressed(WRITE_BUTTON)) {
    Serial.println("Write button pressed");
    if (currentTag.valid) {
      cloneCurrentToTag();
    } else {
      writeSelectedPreset();
    }
  }
}

bool buttonPressed(int pin) {
  ButtonState* button = getButtonState(pin);
  if (button == NULL) return false;

  updateButton(*button);

  if (button->clickReady) {
    button->clickReady = false;
    beepClick();
    return true;
  }

  return false;
}

ButtonState* getButtonState(int pin) {
  if (pin == ENC_SW) return &encoderButton;
  if (pin == READ_BUTTON) return &readButton;
  if (pin == WRITE_BUTTON) return &writeButton;
  return NULL;
}

void updateButton(ButtonState &button) {
  bool reading = digitalRead(button.pin);
  unsigned long now = millis();

  if (reading != button.lastReading) {
    button.lastReading = reading;
    button.lastChange = now;
  }

  if ((now - button.lastChange) < buttonDebounceMs) return;
  if (reading == button.stableState) return;

  button.stableState = reading;

  // Trigger only after a full press and release, avoiding repeated selections while held.
  if (button.stableState == HIGH) {
    button.clickReady = true;
  }
}

// ===================== LCD HELPERS =====================
void lcdPrintTrimmed(const char* text) {
  for (int i = 0; i < 16 && text[i] != '\0'; i++) {
    lcd.print(text[i]);
  }
}

void lcdPrintTrimmedWidth(const char* text, int width) {
  for (int i = 0; i < width && text[i] != '\0'; i++) {
    lcd.print(text[i]);
  }
}

void lcdPrintTagName(const char* label, const char* name) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcdPrintTrimmed(label);
  lcd.setCursor(0, 1);
  lcdPrintTrimmed(name);
}

void lcdPrintFilamentInfo(const char* action, TagData &tag) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.print(action);
  display.print(": ");
  display.print(tag.material);
  display.setCursor(0, 12);
  display.print("Color: ");
  display.print(tag.colorName);
  display.setCursor(0, 24);
  display.print("Nozzle: ");
  display.print(tag.nozzleMin);
  display.print("-");
  display.print(tag.nozzleMax);
  display.print(" C");
  display.setCursor(0, 36);
  display.print("Bed: ");
  display.print(tag.bedMin);
  display.print("-");
  display.print(tag.bedMax);
  display.print(" C");
  display.setCursor(0, 48);
  display.print("Dia: ");
  display.print(tag.diameter / 100.0);
  display.print("mm  ");
  display.print(tag.lengthMeter);
  display.print("m");
  display.display();
}

void oledShowTwoLineMenu(const char* title, const char* value) {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(title);
  display.setTextSize(2);

  String menuValue(value);
  if (menuValue.length() <= 9) {
    display.setCursor(0, 22);
    display.print(">");
    display.print(menuValue);
  } else {
    int splitAt = menuValue.lastIndexOf(' ', 9);
    if (splitAt < 1) splitAt = 9;

    display.setCursor(0, 18);
    display.print(">");
    display.print(menuValue.substring(0, splitAt));
    display.setCursor(12, 40);
    display.print(menuValue.substring(splitAt + (menuValue[splitAt] == ' ' ? 1 : 0)));
  }

  display.display();
}

void oledShowCompactPicker(const char* title, const char* value) {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 8);
  display.print(title);
  display.setCursor(0, 28);
  display.print(">");
  display.print(value);
  display.display();
}

void holdInfoOrDismiss() {
  unsigned long startedAt = millis();

  while (millis() - startedAt < infoDisplayMs) {
    if (digitalRead(ENC_SW) == LOW ||
        digitalRead(READ_BUTTON) == LOW ||
        digitalRead(WRITE_BUTTON) == LOW) {
      Serial.println("Information display dismissed");
      while (digitalRead(ENC_SW) == LOW ||
             digitalRead(READ_BUTTON) == LOW ||
             digitalRead(WRITE_BUTTON) == LOW) {
        delay(10);
      }
      return;
    }
    delay(10);
  }
}

String uidToString(uint8_t* uid, uint8_t uidLength) {
  String text = "";

  for (uint8_t i = 0; i < uidLength; i++) {
    if (i > 0) text += ":";
    if (uid[i] < 0x10) text += "0";
    text += String(uid[i], HEX);
  }

  text.toUpperCase();
  return text;
}

void showUnsupportedMifareTag(uint8_t* uid, uint8_t uidLength) {
  String uidText = uidToString(uid, uidLength);

  currentTag.valid = false;

  Serial.println("MIFARE/Bambu-style tag detected");
  Serial.print("UID length: ");
  Serial.println(uidLength);
  Serial.print("UID: ");
  Serial.println(uidText);
  Serial.println("This sketch can detect this tag, but cannot decode or write Bambu RFID data.");

  beepFail();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.print("Bambu/MIFARE");
  display.setCursor(0, 12);
  display.print("Tag detected");
  display.setCursor(0, 28);
  display.print("UID:");
  display.setCursor(0, 40);
  display.print(uidText);
  display.setCursor(0, 56);
  display.print("Read-only detect");
  display.display();

  holdInfoOrDismiss();
  showMenu();
}

// ===================== MENU DISPLAY =====================
void showMenu() {
  oledShowTwoLineMenu("Menu", menuItems[menuIndex]);
}

void runMenuAction() {
  Serial.print("Run menu action: ");
  Serial.println(menuItems[menuIndex]);

  switch (menuIndex) {
    case 0: selectQuickPreset(); break;
    case 1: readTagToCurrent(); break;
    case 2: cloneCurrentToTag(); break;
    case 3: saveCurrentTag(); break;
    case 4: loadSavedTag(); break;
    case 5: writeSelectedPreset(); break;
    case 6: verifyCurrentTag(); break;
    case 7: showAbout(); break;
  }
}

// ===================== QUICK PRESET SELECT =====================
void selectQuickPreset() {
  Serial.println("Quick Preset started");
  selectMaterialFilter();
  selectedPreset = firstPresetForMaterial(selectedMaterial);

  bool selecting = true;
  int lastPreset = -1;

  while (selecting) {
    if (selectedPreset != lastPreset) {
      char title[22];
      snprintf(title, sizeof(title), "%s Color", materialFilters[selectedMaterial]);
      oledShowCompactPicker(title, presets[selectedPreset].colorName);
      lastPreset = selectedPreset;
    }

    int dir = readEncoderStep();

    if (dir != 0) {
      selectedPreset = nextPresetForMaterial(selectedPreset, selectedMaterial, dir);
      Serial.print("Preset selected: ");
      Serial.println(presets[selectedPreset].name);
      beepTurn();
    }

    if (buttonPressed(ENC_SW)) {
      Serial.print("Preset confirmed: ");
      Serial.println(presets[selectedPreset].name);
      makePresetCurrent(presets[selectedPreset]);
      selecting = false;
    }

    if (buttonPressed(WRITE_BUTTON)) {
      Serial.print("Preset write requested: ");
      Serial.println(presets[selectedPreset].name);
      makePresetCurrent(presets[selectedPreset]);
      cloneCurrentToTag();
      selecting = false;
    }
  }

  showMenu();
}

void selectMaterialFilter() {
  Serial.println("Material selection started");
  bool selecting = true;
  int lastMaterial = -1;

  while (selecting) {
    if (selectedMaterial != lastMaterial) {
      oledShowCompactPicker("Material", materialFilters[selectedMaterial]);
      lastMaterial = selectedMaterial;
    }

    int dir = readEncoderStep();

    if (dir != 0) {

      selectedMaterial += dir;
      if (selectedMaterial < 0) selectedMaterial = materialFilterCount - 1;
      if (selectedMaterial >= materialFilterCount) selectedMaterial = 0;
      Serial.print("Material selected: ");
      Serial.println(materialFilters[selectedMaterial]);

      beepTurn();
    }

    if (buttonPressed(ENC_SW)) {
      Serial.print("Material confirmed: ");
      Serial.println(materialFilters[selectedMaterial]);
      selecting = false;
    }
  }
}

bool presetMatchesMaterial(int presetIndex, int materialIndex) {
  return strcmp(presets[presetIndex].material, materialFilters[materialIndex]) == 0;
}

int firstPresetForMaterial(int materialIndex) {
  for (int i = 0; i < presetCount; i++) {
    if (presetMatchesMaterial(i, materialIndex)) return i;
  }
  return 0;
}

int nextPresetForMaterial(int currentIndex, int materialIndex, int dir) {
  int index = currentIndex;

  for (int tries = 0; tries < presetCount; tries++) {
    index += dir;

    if (index < 0) index = presetCount - 1;
    if (index >= presetCount) index = 0;

    if (presetMatchesMaterial(index, materialIndex)) return index;
  }

  return currentIndex;
}

void makePresetCurrent(FilamentPreset p) {
  Serial.print("Making preset current: ");
  Serial.println(p.name);
  buildPresetTag(p, currentTag);
  currentTag.valid = true;
  strncpy(currentTag.name, p.name, 16);
  currentTag.name[16] = '\0';
  strncpy(currentTag.material, p.material, 16);
  currentTag.material[16] = '\0';
  strncpy(currentTag.colorName, p.colorName, 16);
  currentTag.colorName[16] = '\0';
  currentTag.nozzleMin = p.nozzleMin;
  currentTag.nozzleMax = p.nozzleMax;
  currentTag.bedMin = p.bedMin;
  currentTag.bedMax = p.bedMax;
  currentTag.diameter = p.diameter;
  currentTag.lengthMeter = p.lengthMeter;

  lcdPrintFilamentInfo("Current", currentTag);
  holdInfoOrDismiss();
}

// ===================== READ TAG =====================
void readTagToCurrent() {
  Serial.println("Read tag started");
  uint8_t uid[7];
  uint8_t uidLength;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Place tag...");
  lcd.setCursor(0, 1);
  lcd.print("Reading");

  if (!waitForTag(uid, &uidLength, 10000)) {
    if (tagWaitCanceled) {
      Serial.println("Read tag canceled");
      return;
    }
    Serial.println("Read tag failed: no tag found");
    showError("No tag found");
    return;
  }

  for (int page = 4; page <= 31; page++) {
    uint8_t data[4];

    if (!nfc.ntag2xx_ReadPage(page, data)) {
      Serial.print("Read tag failed on page ");
      Serial.println(page);
      if (page == 4) {
        showUnsupportedMifareTag(uid, uidLength);
        return;
      }
      showError("Read failed");
      return;
    }

    memcpy(currentTag.pages[page], data, 4);
    delay(5);
  }

  currentTag.valid = true;
  decodeTagName(currentTag);
  Serial.print("Read tag successful: ");
  Serial.println(currentTag.name);
  Serial.print("Filament material: ");
  Serial.println(currentTag.material);
  Serial.print("Filament color: ");
  Serial.println(currentTag.colorName);
  Serial.print("Nozzle temp C: ");
  Serial.print(currentTag.nozzleMin);
  Serial.print("-");
  Serial.println(currentTag.nozzleMax);
  Serial.print("Bed temp C: ");
  Serial.print(currentTag.bedMin);
  Serial.print("-");
  Serial.println(currentTag.bedMax);
  Serial.print("Diameter mm x100: ");
  Serial.println(currentTag.diameter);
  Serial.print("Length meter: ");
  Serial.println(currentTag.lengthMeter);

  dumpCurrentToSerial();

  beepSuccess();
  lcdPrintFilamentInfo("Read OK", currentTag);

  holdInfoOrDismiss();
  showMenu();
}

// ===================== CLONE CURRENT =====================
void cloneCurrentToTag() {
  Serial.println("Clone/write current started");
  if (!currentTag.valid) {
    Serial.println("Clone/write failed: no current tag");
    showError("No current tag");
    return;
  }

  uint8_t uid[7];
  uint8_t uidLength;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Clone Current");
  lcd.setCursor(0, 1);
  lcd.print("Place tag");

  if (!waitForTag(uid, &uidLength, 10000)) {
    if (tagWaitCanceled) {
      Serial.println("Clone/write canceled");
      return;
    }
    Serial.println("Clone/write failed: no tag found");
    showError("No tag found");
    return;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Writing clone");
  lcd.setCursor(0, 1);
  lcdPrintTrimmed(currentTag.name);

  bool ok = writeTagData(currentTag);

  if (ok) {
    Serial.print("Clone/write successful: ");
    Serial.println(currentTag.name);
    Serial.print("Filament material: ");
    Serial.println(currentTag.material);
    Serial.print("Filament color: ");
    Serial.println(currentTag.colorName);
    Serial.print("Nozzle temp C: ");
    Serial.print(currentTag.nozzleMin);
    Serial.print("-");
    Serial.println(currentTag.nozzleMax);
    Serial.print("Bed temp C: ");
    Serial.print(currentTag.bedMin);
    Serial.print("-");
    Serial.println(currentTag.bedMax);
    beepSuccess();
    lcdPrintFilamentInfo("Write OK", currentTag);
  } else {
    Serial.println("Clone/write failed during write or verify");
    showError("Clone failed");
    return;
  }

  holdInfoOrDismiss();
  showMenu();
}

// ===================== SAVE / LOAD FLASH =====================
void saveCurrentTag() {
  Serial.println("Save current tag started");
  if (!currentTag.valid) {
    Serial.println("Save failed: no current tag");
    showError("No current tag");
    return;
  }

  int slot = selectSaveSlot("Save Slot");
  selectedSaveSlot = slot;

  prefs.putBytes(slotKey("data", slot).c_str(), currentTag.pages, sizeof(currentTag.pages));
  prefs.putString(slotKey("name", slot).c_str(), currentTag.name);
  prefs.putString(slotKey("mat", slot).c_str(), currentTag.material);
  prefs.putString(slotKey("color", slot).c_str(), currentTag.colorName);
  prefs.putInt(slotKey("nmin", slot).c_str(), currentTag.nozzleMin);
  prefs.putInt(slotKey("nmax", slot).c_str(), currentTag.nozzleMax);
  prefs.putInt(slotKey("bmin", slot).c_str(), currentTag.bedMin);
  prefs.putInt(slotKey("bmax", slot).c_str(), currentTag.bedMax);
  prefs.putInt(slotKey("dia", slot).c_str(), currentTag.diameter);
  prefs.putInt(slotKey("len", slot).c_str(), currentTag.lengthMeter);
  prefs.putBool(slotKey("valid", slot).c_str(), true);
  Serial.print("Saved current tag: ");
  Serial.println(currentTag.name);
  Serial.print("Saved slot: ");
  Serial.println(slot + 1);

  beepSuccess();
  char label[17];
  snprintf(label, sizeof(label), "Saved Slot %02d", slot + 1);
  lcdPrintTagName(label, currentTag.name);

  delay(shortStatusMs);
  showMenu();
}

void loadSavedTag() {
  Serial.println("Load saved tag started");
  int slot = selectSaveSlot("Load Slot");
  selectedSaveSlot = slot;
  bool valid = prefs.getBool(slotKey("valid", slot).c_str(), false);

  if (!valid) {
    Serial.println("Load failed: no saved tag");
    showError("Empty slot");
    return;
  }

  prefs.getBytes(slotKey("data", slot).c_str(), currentTag.pages, sizeof(currentTag.pages));
  String savedName = prefs.getString(slotKey("name", slot).c_str(), "Saved Tag");
  String savedMaterial = prefs.getString(slotKey("mat", slot).c_str(), "Unknown");
  String savedColor = prefs.getString(slotKey("color", slot).c_str(), "Unknown");
  currentTag.nozzleMin = prefs.getInt(slotKey("nmin", slot).c_str(), 0);
  currentTag.nozzleMax = prefs.getInt(slotKey("nmax", slot).c_str(), 0);
  currentTag.bedMin = prefs.getInt(slotKey("bmin", slot).c_str(), 0);
  currentTag.bedMax = prefs.getInt(slotKey("bmax", slot).c_str(), 0);
  currentTag.diameter = prefs.getInt(slotKey("dia", slot).c_str(), 0);
  currentTag.lengthMeter = prefs.getInt(slotKey("len", slot).c_str(), 0);

  strncpy(currentTag.name, savedName.c_str(), 16);
  currentTag.name[16] = '\0';
  strncpy(currentTag.material, savedMaterial.c_str(), 16);
  currentTag.material[16] = '\0';
  strncpy(currentTag.colorName, savedColor.c_str(), 16);
  currentTag.colorName[16] = '\0';
  currentTag.valid = true;
  if (strcmp(currentTag.material, "Unknown") == 0 || strcmp(currentTag.colorName, "Unknown") == 0) {
    decodeTagName(currentTag);
  }
  if (currentTag.nozzleMin == 0 && currentTag.nozzleMax == 0) {
    decodeTagSettings(currentTag);
  }
  Serial.print("Loaded saved tag: ");
  Serial.println(currentTag.name);
  Serial.print("Loaded slot: ");
  Serial.println(slot + 1);

  beepSuccess();
  lcdPrintFilamentInfo("Loaded", currentTag);

  holdInfoOrDismiss();
  showMenu();
}

int selectSaveSlot(const char* title) {
  bool selecting = true;
  int lastSlot = -1;

  while (selecting) {
    if (selectedSaveSlot != lastSlot) {
      String savedName = prefs.getString(slotKey("name", selectedSaveSlot).c_str(), "Empty");

      char slotTitle[22];
      snprintf(slotTitle, sizeof(slotTitle), "%s %02d", title, selectedSaveSlot + 1);
      oledShowCompactPicker(slotTitle, savedName.c_str());

      Serial.print(title);
      Serial.print(": ");
      Serial.print(selectedSaveSlot + 1);
      Serial.print(" ");
      Serial.println(savedName);

      lastSlot = selectedSaveSlot;
    }

    int dir = readEncoderStep();

    if (dir != 0) {
      selectedSaveSlot += dir;
      if (selectedSaveSlot < 0) selectedSaveSlot = saveSlotCount - 1;
      if (selectedSaveSlot >= saveSlotCount) selectedSaveSlot = 0;

      beepTurn();
    }

    if (buttonPressed(ENC_SW)) {
      Serial.print("Slot confirmed: ");
      Serial.println(selectedSaveSlot + 1);
      return selectedSaveSlot;
    }
  }

  return selectedSaveSlot;
}

String slotKey(const char* prefix, int slot) {
  String key = prefix;
  key += slot;
  return key;
}

// ===================== WRITE PRESET =====================
void writeSelectedPreset() {
  Serial.print("Write selected preset: ");
  Serial.println(presets[selectedPreset].name);
  makePresetCurrent(presets[selectedPreset]);
  cloneCurrentToTag();
}

// ===================== VERIFY CURRENT =====================
void verifyCurrentTag() {
  Serial.println("Verify current tag started");
  if (!currentTag.valid) {
    Serial.println("Verify failed: no current tag");
    showError("No current tag");
    return;
  }

  uint8_t uid[7];
  uint8_t uidLength;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Verify Current");
  lcd.setCursor(0, 1);
  lcd.print("Place tag");

  if (!waitForTag(uid, &uidLength, 10000)) {
    if (tagWaitCanceled) {
      Serial.println("Verify canceled");
      return;
    }
    Serial.println("Verify failed: no tag found");
    showError("No tag found");
    return;
  }

  for (int page = 4; page <= 31; page++) {
    uint8_t data[4];

    if (!nfc.ntag2xx_ReadPage(page, data)) {
      Serial.print("Verify read failed page ");
      Serial.println(page);
      showError("Read failed");
      return;
    }

    for (int i = 0; i < 4; i++) {
      if (data[i] != currentTag.pages[page][i]) {
        Serial.print("Verify mismatch page ");
        Serial.print(page);
        Serial.print(" byte ");
        Serial.println(i);
        showError("Verify failed");
        return;
      }
    }
  }

  beepSuccess();
  Serial.print("Verify successful: ");
  Serial.println(currentTag.name);
  lcdPrintFilamentInfo("Verify OK", currentTag);

  holdInfoOrDismiss();
  showMenu();
}

// ===================== WAIT FOR TAG =====================
bool waitForTag(uint8_t* uid, uint8_t* uidLength, unsigned long timeoutMs) {
  Serial.println("Waiting for NFC tag");
  tagWaitCanceled = false;
  unsigned long start = millis();
  unsigned long encoderPressedAt = 0;
  bool encoderWasPressed = false;

  while (millis() - start < timeoutMs) {
    if (digitalRead(ENC_SW) == LOW) {
      if (!encoderWasPressed) {
        encoderWasPressed = true;
        encoderPressedAt = millis();
      }

      if (millis() - encoderPressedAt >= 900) {
        Serial.println("Tag wait canceled by encoder long press");
        tagWaitCanceled = true;
        beepFail();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Canceled");
        lcd.setCursor(0, 1);
        lcd.print("Main menu");
        delay(900);
        while (digitalRead(ENC_SW) == LOW) {
          delay(10);
        }
        encoderButton.stableState = HIGH;
        encoderButton.lastReading = HIGH;
        encoderButton.clickReady = false;
        encoderButton.lastChange = millis();
        showMenu();
        return false;
      }
    } else {
      encoderWasPressed = false;
    }

    bool success = nfc.readPassiveTargetID(
      PN532_MIFARE_ISO14443A,
      uid,
      uidLength,
      250
    );

    if (success) {
      Serial.print("Tag found UID:");
      for (int i = 0; i < *uidLength; i++) {
        Serial.print(" ");
        if (uid[i] < 0x10) Serial.print("0");
        Serial.print(uid[i], HEX);
      }
      Serial.println();
      return true;
    }
  }

  Serial.println("Tag wait timed out");
  return false;
}

// ===================== WRITE TAG DATA =====================
bool writeTagData(TagData tag) {
  for (int page = 4; page <= 31; page++) {
    bool ok = nfc.ntag2xx_WritePage(page, tag.pages[page]);

    if (!ok) {
      Serial.print("Write failed page ");
      Serial.println(page);
      return false;
    }

    delay(20);

    uint8_t verifyData[4];

    if (!nfc.ntag2xx_ReadPage(page, verifyData)) {
      Serial.print("Verify read failed page ");
      Serial.println(page);
      return false;
    }

    for (int i = 0; i < 4; i++) {
      if (verifyData[i] != tag.pages[page][i]) {
        Serial.print("Verify mismatch page ");
        Serial.println(page);
        return false;
      }
    }
  }

  return true;
}

// ===================== BUILD PRESET TAG =====================
void buildPresetTag(FilamentPreset p, TagData &tag) {
  Serial.print("Building preset tag data for ");
  Serial.println(p.name);
  memset(tag.pages, 0, sizeof(tag.pages));

  uint8_t headerData[4] = {0x7B, 0x00, 0x65, 0x00};
  memcpy(tag.pages[4], headerData, 4);

  writeStringToPages(tag, 5, p.sku, 20);
  writeStringToPages(tag, 10, "AC", 20);
  writeStringToPages(tag, 15, p.material, 20);

  uint8_t colorData[4];
  hexToByte(p.colorHex, colorData, 4);
  reverseArray(colorData, 4);
  memcpy(tag.pages[20], colorData, 4);

  uint8_t nozzleData[4];
  makeTwoIntPage(p.nozzleMin, p.nozzleMax, nozzleData);
  memcpy(tag.pages[24], nozzleData, 4);

  uint8_t bedData[4];
  makeTwoIntPage(p.bedMin, p.bedMax, bedData);
  memcpy(tag.pages[29], bedData, 4);

  uint8_t filamentData[4];
  makeTwoIntPage(p.diameter, p.lengthMeter, filamentData);
  memcpy(tag.pages[30], filamentData, 4);

  uint8_t fixedData[4] = {0xE8, 0x03, 0x00, 0x00};
  memcpy(tag.pages[31], fixedData, 4);
}

void writeStringToPages(TagData &tag, int startPage, const char* text, int maxBytes) {
  uint8_t buffer[20];
  memset(buffer, 0, sizeof(buffer));

  int len = strlen(text);
  if (len > maxBytes) len = maxBytes;

  for (int i = 0; i < len; i++) {
    buffer[i] = text[i];
  }

  for (int i = 0; i < maxBytes / 4; i++) {
    memcpy(tag.pages[startPage + i], &buffer[i * 4], 4);
  }
}

void makeTwoIntPage(int a, int b, uint8_t output[4]) {
  output[0] = a & 0xFF;
  output[1] = (a >> 8) & 0xFF;
  output[2] = b & 0xFF;
  output[3] = (b >> 8) & 0xFF;
}

// ===================== DECODE TAG NAME =====================
void decodeTagName(TagData &tag) {
  char material[17];
  memset(material, 0, sizeof(material));

  int index = 0;

  for (int page = 15; page <= 18; page++) {
    for (int i = 0; i < 4; i++) {
      char c = tag.pages[page][i];
      if (c >= 32 && c <= 126 && index < 16) {
        material[index++] = c;
      }
    }
  }

  if (strlen(material) > 0) {
    strncpy(tag.material, material, 16);
    tag.material[16] = '\0';
  } else {
    strcpy(tag.material, "Unknown");
  }

  decodeTagColor(tag);
  decodeTagSettings(tag);

  if (strcmp(tag.colorName, "Unknown") == 0) {
    strncpy(tag.name, tag.material, 16);
  } else {
    snprintf(tag.name, sizeof(tag.name), "%s %s", tag.material, tag.colorName);
  }
  tag.name[16] = '\0';
}

void decodeTagSettings(TagData &tag) {
  tag.nozzleMin = readTwoByteInt(tag.pages[24][0], tag.pages[24][1]);
  tag.nozzleMax = readTwoByteInt(tag.pages[24][2], tag.pages[24][3]);
  tag.bedMin = readTwoByteInt(tag.pages[29][0], tag.pages[29][1]);
  tag.bedMax = readTwoByteInt(tag.pages[29][2], tag.pages[29][3]);
  tag.diameter = readTwoByteInt(tag.pages[30][0], tag.pages[30][1]);
  tag.lengthMeter = readTwoByteInt(tag.pages[30][2], tag.pages[30][3]);
}

int readTwoByteInt(uint8_t lowByte, uint8_t highByte) {
  return lowByte | (highByte << 8);
}

void decodeTagColor(TagData &tag) {
  uint8_t colorBytes[4];
  memcpy(colorBytes, tag.pages[20], 4);
  reverseArray(colorBytes, 4);

  char colorHex[9];
  snprintf(colorHex, sizeof(colorHex), "%02X%02X%02X%02X", colorBytes[0], colorBytes[1], colorBytes[2], colorBytes[3]);

  strcpy(tag.colorName, "Unknown");

  for (int i = 0; i < presetCount; i++) {
    if (strcmp(presets[i].material, tag.material) == 0 && strcmp(presets[i].colorHex, colorHex) == 0) {
      strncpy(tag.colorName, presets[i].colorName, 16);
      tag.colorName[16] = '\0';
      return;
    }
  }

  for (int i = 0; i < presetCount; i++) {
    if (strcmp(presets[i].colorHex, colorHex) == 0) {
      strncpy(tag.colorName, presets[i].colorName, 16);
      tag.colorName[16] = '\0';
      return;
    }
  }
}

// ===================== SERIAL DUMP =====================
void dumpCurrentToSerial() {
  Serial.println();
  Serial.println("===== CURRENT TAG DATA =====");
  Serial.print("Material: ");
  Serial.println(currentTag.material);
  Serial.print("Color: ");
  Serial.println(currentTag.colorName);
  Serial.print("Nozzle: ");
  Serial.print(currentTag.nozzleMin);
  Serial.print("-");
  Serial.print(currentTag.nozzleMax);
  Serial.println(" C");
  Serial.print("Bed: ");
  Serial.print(currentTag.bedMin);
  Serial.print("-");
  Serial.print(currentTag.bedMax);
  Serial.println(" C");
  Serial.print("Diameter: ");
  Serial.print(currentTag.diameter / 100.0);
  Serial.println(" mm");
  Serial.print("Length: ");
  Serial.print(currentTag.lengthMeter);
  Serial.println(" m");

  for (int page = 4; page <= 31; page++) {
    Serial.print("Page ");
    if (page < 10) Serial.print("0");
    Serial.print(page);
    Serial.print(": ");

    for (int i = 0; i < 4; i++) {
      if (currentTag.pages[page][i] < 0x10) Serial.print("0");
      Serial.print(currentTag.pages[page][i], HEX);
      Serial.print(" ");
    }

    Serial.print(" | ");

    for (int i = 0; i < 4; i++) {
      char c = currentTag.pages[page][i];
      if (c >= 32 && c <= 126) Serial.print(c);
      else Serial.print(".");
    }

    Serial.println();
  }

  Serial.println("============================");
}

// ===================== HEX HELPERS =====================
void hexToByte(const char* hexString, uint8_t* byteArray, int byteCount) {
  for (int i = 0; i < byteCount; i++) {
    char hexPair[3];
    hexPair[0] = hexString[i * 2];
    hexPair[1] = hexString[i * 2 + 1];
    hexPair[2] = '\0';
    byteArray[i] = strtol(hexPair, NULL, 16);
  }
}

void reverseArray(uint8_t* array, int len) {
  for (int i = 0; i < len / 2; i++) {
    uint8_t temp = array[i];
    array[i] = array[len - 1 - i];
    array[len - 1 - i] = temp;
  }
}

// ===================== ABOUT =====================
void showAbout() {
  Serial.println("Showing About screen");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ACE RFID Box");
  lcd.setCursor(0, 1);
  lcd.print("ESP32 SH1106");
  delay(2200);
  showMenu();
}

// ===================== BUZZER / LED =====================
void playTone(int frequency, int durationMs) {
  if (BUZZER_IS_ACTIVE) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(durationMs);
    digitalWrite(BUZZER_PIN, LOW);
  } else {
    ledcAttach(BUZZER_PIN, frequency, 10);
    ledcWriteTone(BUZZER_PIN, frequency);
    delay(durationMs);
    ledcWriteTone(BUZZER_PIN, 0);
  }
}

void buzzerOff() {
  if (BUZZER_IS_ACTIVE) {
    digitalWrite(BUZZER_PIN, LOW);
  } else {
    ledcWriteTone(BUZZER_PIN, 0);
  }
}

void beepTurn() {
  Serial.println("Encoder turned");
  playTone(1200, 25);
}

void beepClick() {
  Serial.println("Button click");
  playTone(1800, 50);
}

void beepSuccess() {
  Serial.println("Success feedback");
  digitalWrite(LED_BUILTIN, LOW);
  delay(80);

  digitalWrite(LED_PIN, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);
  playTone(1800, 130);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  delay(80);

  digitalWrite(LED_PIN, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);
  playTone(2400, 160);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(LED_BUILTIN, HIGH);
}

void beepFail() {
  Serial.println("Failure feedback");
  playTone(500, 550);
}

// ===================== ERROR DISPLAY =====================
void showError(const char* msg) {
  Serial.print("ERROR: ");
  Serial.println(msg);
  beepFail();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ERROR");
  lcd.setCursor(0, 1);
  lcdPrintTrimmed(msg);

  delay(2200);
  showMenu();
}
