// TEA7567 setup
#include <Wire.h>
#include <TEA5767.h>

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
float foundStations[10];
int stationCount;
double currentFreq = 90.9;

// SSD1306 setup
#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>

// U8G2_SSD1306_128X64_NONAME_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
U8G2_SH1106_128X64_NONAME_2_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);


// Rotary encoder setup
#include <Encoder.h>

Encoder enc(3,2);
long encPos = 0;
const unsigned long debounceMs = 250;
unsigned long lastEventMs = 0;
const int countPerDetent = 4;
const double mhzInc = 0.2;

// EEPROM setup for storing bookmarks and last freq. value
#include <EEPROM.h>
#define SETTINGS_ADDR 0
const long settings_id = 0xCAFE;
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
  loadSuccess = load_settings();
  if (!loadSuccess) {
    Serial.println("Could not load settings.");
  }

  // RADIO
  Wire.begin();
  radio.awake();
  currentFreq = settings.lastFreq;
  radio.setFrequency(currentFreq);

  // DISPLAY
  u8g2.begin();

}

void loop() {
  long newPos = enc.read();
  if (newPos != encPos) {
    unsigned long t = millis();
    if (t - lastEventMs >= debounceMs) {
      // Increment current frequency by number of detents
      double freqInc = (newPos > encPos) ? ceil(double(newPos - encPos) / countPerDetent) : floor(double(newPos - encPos) / countPerDetent);
      double newFreq = currentFreq + mhzInc * freqInc;
      Serial.print("Enc delta: ");
      Serial.println(double(newPos - encPos)/countPerDetent);
      Serial.print("New freq: ");
      Serial.println(newFreq);
      // Make sure new frequency is valid
      if (newFreq <= FM_MAX && newFreq >= FM_MIN) {
        currentFreq = newFreq;
        radio.setFrequency(currentFreq);
        settings.lastFreq = currentFreq;
        save_settings();
      }
      lastEventMs = millis();
    }
    encPos = newPos;
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
  } while ( u8g2.nextPage() );
}


void save_settings() {
  EEPROM.put(SETTINGS_ADDR, settings);
}

bool load_settings() {
  // Returns True if settings successfully loaded
  radio_settings_t s = EEPROM.get(SETTINGS_ADDR, s);
  Serial.println("Settings loaded:");
  Serial.println(s.lastFreq);
  Serial.println(s.bookmarksLen);
  Serial.println(s.bookmarks[0]);
  Serial.println(s.id);
  if (s.id == settings_id) {
    settings = s;
    return true;
  } else {
    // If settings invalid, initialize with default values
    s.lastFreq = 90.9;
    s.bookmarksLen = 0;
    // Fill bookmarks array with zeros
    memset(s.bookmarks, 0.0, sizeof(s.bookmarks));
    s.id = settings_id;
    settings = s;
    save_settings();
    return false;
  }
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