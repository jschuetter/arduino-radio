#include "TEA5767.h"
#include "tea5767_core.h"
#include <stdint.h>
#include <Arduino.h>
#include <Wire.h>

void tea5767_i2c_read(uint8_t addr, uint8_t *data, uint8_t len) {
  uint8_t count = Wire.requestFrom((int)addr, (int)len);
  if (count != len) {
    Serial.println("tea5767_i2c_read error: didn't get expected bytes");
    return;
  }
  for (uint8_t i = 0; i < len; i++) {
    if (Wire.available()) {
      data[i] = Wire.read();
    }
  }
}

void tea5767_i2c_write(uint8_t addr, const uint8_t *data, uint8_t len) {
  Wire.beginTransmission(addr);
  for (uint8_t i = 0; i < len; i++) {
    Wire.write(data[i]);
  }
  uint8_t result = Wire.endTransmission();
  delay(10);
  if (result != 0) {
    Serial.print("tea5767_i2c_write error: ");
    Serial.println(result, 10);
    return;
  }
}

void tea5767_delay(uint16_t ms) {
  delay(ms);
}

void tea5767_print(const char *msg) {
  Serial.println(msg);
}

TEA5767::TEA5767(void (*displayInfo)(const tea5767_info_t *),
                 void (*updateConfig)(tea5767_config_t *)) {
  displayInfoCallback = displayInfo;
  tea5767_config_t config;
  tea5767_get_default_config(&config);
  if (updateConfig) {
    updateConfig(&config);
  }
  tea5767_configure(&config);
}

void TEA5767::awake() {
  tea5767_awake();
  displayInfo();
}

void TEA5767::getInfo(tea5767_info_t *info) {
  tea5767_get_info(info);
}

void TEA5767::mute() {
  tea5767_mute();
  displayInfo();
}

void TEA5767::searchDown() {
  displayInfo(true);
  tea5767_search(TEA5767_SEARCH_DOWN);
  displayInfo();
}

void TEA5767::searchUp() {
  displayInfo(true);
  tea5767_search(TEA5767_SEARCH_UP);
  displayInfo();
}

void TEA5767::setFrequency(float mhz) {
  tea5767_set_frequency(mhz);
  displayInfo();
}

void TEA5767::standby() {
  tea5767_standby();
  displayInfo();
}

void TEA5767::unmute() {
  tea5767_unmute();
  displayInfo();
}

void TEA5767::displayInfo(bool searching) {
  if (displayInfoCallback != nullptr) {
    tea5767_info_t info;
    tea5767_get_info(&info);
    info.searching = searching;
    displayInfoCallback(&info);
  }
}
