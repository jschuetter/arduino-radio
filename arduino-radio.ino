// TEA5767
#include <Wire.h>
#include <TEA5767.h>
// SSD1306
#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>
// Other includes
#include <Encoder.h>
#include <EEPROM.h>


// TEA5767 setup
#define FM_MIN 88.0
#define FM_MAX 108.0

// Optional: this callback function can be passed to the TEA5767 constructor.
// It is invoked whenever the radio state is updated.
// You can use it to display the current information on an external
// module (e.g., OLED/LCD) or simply log it to the Serial monitor
// for debugging, as shown here.
void radioPrintInfo(const tea5767_info_t *info);

// Optional: this callback function can be passed to the TEA5767 constructor.
// You can override only the settings you care about here, leaving
// all other values at their defaults. For example, you might want
// to change the band limits, mute setting, or de-emphasis time.
// This way, you avoid having to manually initialize every property.
void radioUpdateConfig(tea5767_config_t *config);

TEA5767 radio(&radioPrintInfo, &radioUpdateConfig);
double currentFreq;

// SSD1306 setup
// U8G2_SSD1306_128X64_NONAME_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
U8G2_SH1106_128X64_NONAME_2_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);


// Rotary encoder setup
#define BTN_PIN 4
Encoder enc(3,2);
long encPos = 0;
bool lastBtnRead = HIGH;
bool btnState = HIGH;
long lastBtnEvent = 0;
long tPress = 0;
bool toggleDone = false;
const unsigned long DebounceMs = 250;
const unsigned long HoldMs = 1000;
unsigned long lastEventMs = 0;
const int CountPerDetent = 4;
const double MhzInc = 0.2;

// EEPROM setup for storing bookmarks and last freq. value
#define SETTINGS_ADDR 0
const long SettingsId = 0xCAFE;
const int MaxBookmarks = 10;
bool bookmarkMode = false;
int currentBookmark = 0;
struct radio_settings_t {
  double lastFreq;
  int bookmarksLen;
  double bookmarks[10];
  long id;
};
radio_settings_t settings;

bool loadSuccess;
void setup() {
  // Get serial output
  Serial.begin(9600);

  // Load settings from EEPROM
  loadSuccess = loadSettings();
  if (!loadSuccess) {
    Serial.println("Could not load settings.");
  }

  // RADIO
  Wire.begin();
  radio.awake();
  currentFreq = settings.lastFreq;
  radio.setFrequency(currentFreq);
  pinMode(BTN_PIN, INPUT_PULLUP); // Rotary btn pin

  // DISPLAY
  u8g2.begin();

}

void loop() {
  unsigned long t = millis();
  long newPos = enc.read();
  bool btnRead = digitalRead(BTN_PIN);

  if (newPos != encPos) {
    if (t - lastEventMs >= DebounceMs) {
      if (!bookmarkMode) {
        // Increment current frequency by number of detents
        double freqInc = (newPos > encPos) ? ceil(double(newPos - encPos) / CountPerDetent) : floor(double(newPos - encPos) / CountPerDetent);
        double newFreq = currentFreq + MhzInc * freqInc;
        Serial.print("Enc delta: ");
        Serial.println(double(newPos - encPos)/CountPerDetent);
        Serial.print("New freq: ");
        Serial.println(newFreq);
        // Make sure new frequency is valid
        if (newFreq <= FM_MAX && newFreq >= FM_MIN) {
          currentFreq = newFreq;
          radio.setFrequency(currentFreq);
          settings.lastFreq = currentFreq;
          saveSettings();
        }
      } else {
        // Bookmark mode: increment/decrement 1 bookmark
        if (newPos > encPos) currentBookmark++;
        else currentBookmark--;
        
        // Wrap indices
        if (currentBookmark > settings.bookmarksLen-1) currentBookmark = 0;
        else if (currentBookmark < 0) currentBookmark = settings.bookmarksLen-1;

        currentFreq = settings.bookmarks[currentBookmark];
        radio.setFrequency(currentFreq);
        settings.lastFreq = currentFreq;
        saveSettings();
      }
      lastEventMs = millis();
    }
    encPos = newPos;
  }

  if (btnRead != lastBtnRead) {
    lastBtnEvent = t;
    lastBtnRead = btnRead;
  }
  if (t - lastBtnEvent >= DebounceMs && btnRead != btnState) {
    btnState = btnRead;
    if (btnState == LOW) {
      tPress = t;
      Serial.println("Btn press");
    } else {
      // Btn released
      if (!toggleDone && settings.bookmarksLen > 0) {
        bookmarkMode = !bookmarkMode;
      } else toggleDone = false; 
    }
  }
  if (btnState == LOW && t - tPress >= HoldMs && !toggleDone) {
    Serial.println("Btn hold");
    (isBookmark()) ? removeBookmark() : addBookmark();
    toggleDone = true;
    Serial.println(settings.bookmarksLen);
    for (int i = 0; i < MaxBookmarks; i++) {
      Serial.print(settings.bookmarks[i]);
      Serial.print(", ");
    }
    Serial.println();
  }
  
  char freqStr[5];
  dtostrf(currentFreq, 3, 1, freqStr);
  char freq[15];
  sprintf(freq, "%s MHz", freqStr);
  
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_osr35_tn);
    u8g2.drawStr(64-(u8g2.getStrWidth(freqStr)/2),50,freqStr);
    u8g2.setFont(u8g2_font_timR10_tr);
    u8g2.drawStr(98,64,"MHz");
    if (bookmarkMode) {
      u8g2.drawStr(60, 12, "Bookmarks");
    }
    if (isBookmark()) {
      u8g2.setFont(u8g2_font_twelvedings_t_all);
      u8g2.drawStr(2, 15, "B");
    }
  } while ( u8g2.nextPage() );
}


void saveSettings() {
  EEPROM.put(SETTINGS_ADDR, settings);
}

bool loadSettings() {
  // Returns True if settings successfully loaded
  radio_settings_t s = EEPROM.get(SETTINGS_ADDR, s);
  Serial.println("Settings loaded:");
  Serial.println(s.lastFreq);
  Serial.println(s.bookmarksLen);
  Serial.println(s.bookmarks[0]);
  Serial.println(s.id);
  if (s.id == SettingsId) {
    settings = s;
    return true;
  } else {
    // If settings invalid, initialize with default values
    s.lastFreq = 90.9;
    s.bookmarksLen = 0;
    // Fill bookmarks array with zeros
    memset(s.bookmarks, 0.0, sizeof(s.bookmarks));
    s.id = SettingsId;
    settings = s;
    saveSettings();
    return false;
  }
}

bool addBookmark() {
  // Return value = success
  if (isBookmark()) return false;
  Serial.println(settings.bookmarksLen);
  Serial.println(MaxBookmarks);
  Serial.println(settings.bookmarksLen < MaxBookmarks);
  if (settings.bookmarksLen < MaxBookmarks) {
    settings.bookmarks[settings.bookmarksLen] = currentFreq;
    settings.bookmarksLen++;
    Serial.println("Bookmark added");
    sortBookmarks();
    saveSettings();
    return true;
  } else return false;
}

bool removeBookmark() {
  // Return value = success
  for (int i = 0; i < settings.bookmarksLen; i++) {
    if (settings.bookmarks[i] == currentFreq) {
      settings.bookmarks[i] = 0.0;
      sortBookmarks();
      saveSettings();
      if (settings.bookmarksLen == 0 && bookmarkMode) bookmarkMode = false;
      return true;
    }
  }
  return false;
}

bool isBookmark() {
  for (int i = 0; i < settings.bookmarksLen; i++) {
    if (settings.bookmarks[i] == currentFreq) return true;
  }
  return false;
}

void sortBookmarks() {
  // Modified bubble sort algorithm
  int i = 0;
  while (i < MaxBookmarks) {
    Serial.print(i);
    Serial.print(" ");
    double val = settings.bookmarks[i];
    Serial.println(val);
    int newIdx = i+1;
    if (val == 0.0) newIdx = MaxBookmarks-1;
    while (
      (val > settings.bookmarks[newIdx] && settings.bookmarks[newIdx] != 0.0) 
      && newIdx < MaxBookmarks-1
      ) {
      newIdx++;
      Serial.print("newIdx ");
      Serial.println(newIdx);
    }
    for (int j = i+1; j < newIdx; j++) {
      settings.bookmarks[j-1] = settings.bookmarks[j];
    }
    i++;
    settings.bookmarks[newIdx-1] = val;
  }

  int len = 0;
  for (int i = 0; i < MaxBookmarks; i++) {
    if (settings.bookmarks[i] != 0.0) len++;
  }
  settings.bookmarksLen = len;
}

void radioPrintInfo(const tea5767_info_t *info) {
  Serial.println("**********");
  if (info->muted) {
    Serial.println("Muted!");
  } else if (info->standby) {
    Serial.println("Standby!");
  } else {
    if (info->searching) {
      Serial.println("Searching...");
    } else {
      Serial.print(info->mhz);
      Serial.println("MHz");
    }
  }
  Serial.print("Stereo: ");
  Serial.println(info->stereo ? "Stereo" : "Mono");
  Serial.print("Tune: ");
  Serial.println(info->tune);
  Serial.print("Signal level: ");
  Serial.print(info->signal_level);
  Serial.println("%");
  Serial.println("**********");
}

void radioUpdateConfig(tea5767_config_t *config) {
  config->standby = false;
}