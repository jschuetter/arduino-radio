// TEA7567 setup
#include <Wire.h>
#include <TEA5767.h>

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

// SSD1306 setup

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
static const unsigned char PROGMEM logo_bmp[] =
{ 0b00000000, 0b11000000,
  0b00000001, 0b11000000,
  0b00000001, 0b11000000,
  0b00000011, 0b11100000,
  0b11110011, 0b11100000,
  0b11111110, 0b11111000,
  0b01111110, 0b11111111,
  0b00110011, 0b10011111,
  0b00011111, 0b11111100,
  0b00001101, 0b01110000,
  0b00011011, 0b10100000,
  0b00111111, 0b11100000,
  0b00111111, 0b11110000,
  0b01111100, 0b11110000,
  0b01110000, 0b01110000,
  0b00000000, 0b00110000 };


void setup() {
  Serial.begin(9600);
  Wire.begin();
  radio.awake();
  radio.setFrequency(90.9);
}

void loop() {
  
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
