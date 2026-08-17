#include "tea5767_core.h"

#pragma once

class TEA5767 {
public:
  TEA5767(void (*displayInfo)(const tea5767_info_t *) = nullptr,
          void (*updateConfig)(tea5767_config_t *) = nullptr);
  void awake();
  void getInfo(tea5767_info_t *info);
  void mute();
  void searchDown();
  void searchUp();
  void setFrequency(float mhz);
  void standby();
  void unmute();

private:
  void displayInfo(bool searching = false);
  void (*displayInfoCallback)(const tea5767_info_t *) = nullptr;
};