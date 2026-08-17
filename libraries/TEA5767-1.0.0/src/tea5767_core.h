#include <stdint.h>
#include <stdbool.h>

#ifndef TEA5767_H
#define TEA5767_H

#define TEA5767_IC_ADDRESS 0x60
#define TEA5767_US_EUROPE_BAND_MIN 87.5f
#define TEA5767_US_EUROPE_BAND_MAX 108.0f
#define TEA5767_JAPANESE_BAND_MIN 76.0f
#define TEA5767_JAPANESE_BAND_MAX 91.0f
#define TEA5767_IF_OFFSET_HZ 225000ul
#define TEA5767_MAX_IF_COUNTER 128
#define TEA5767_MAX_ADC 15

typedef enum {
  TEA5767_SEARCH_DOWN,
  TEA5767_SEARCH_UP
} tea5767_sud_t;

typedef enum {
  TEA5767_SEARCH_STOP_LEVEL_NONE,
  TEA5767_SEARCH_STOP_LEVEL_LOW,
  TEA5767_SEARCH_STOP_LEVEL_MID,
  TEA5767_SEARCH_STOP_LEVEL_HIGH
} tea5767_ssl_t;

typedef enum {
  TEA5767_SIDE_INJECTION_LOW,
  TEA5767_SIDE_INJECTION_HIGH
} tea5767_hlsi_t;

typedef enum {
  TEA5767_BAND_LIMIT_US_EUROPE,
  TEA5767_BAND_LIMIT_JAPANESE
} tea5767_bl_t;

typedef enum {
  TEA5767_CLOCK_13MHZ,
  TEA5767_CLOCK_32768HZ,
  TEA5767_CLOCK_6500KHZ
} tea5767_clock_t;

typedef enum {
  TEA5767_DE_EMPHASIS_TIME_CONSTANT_50US,
  TEA5767_DE_EMPHASIS_TIME_CONSTANT_75US
} tea5767_dtc_t;

typedef struct
{
  bool searching;
  bool muted;
  bool standby;
  bool ready;
  bool band_limit_reached;
  float mhz;
  bool stereo;
  int8_t tune;
  uint8_t signal_level;
  uint8_t chip_id;
} tea5767_info_t;

typedef struct
{
  bool mute;
  bool search_mode;
  float mhz;
  tea5767_sud_t search;
  tea5767_ssl_t search_stop_level;
  tea5767_hlsi_t side_injection;
  bool mono_to_stereo;
  bool mute_right;
  bool mute_left;
  bool software_programmable_port1;
  bool software_programmable_port2;
  bool standby;
  tea5767_bl_t band_limits;
  tea5767_clock_t clock_frequency;
  bool soft_mute;
  bool high_cut_control;
  bool stereo_noise_cancelling;
  bool search_indicator;
  tea5767_dtc_t de_emphasis_time_constant;
} tea5767_config_t;

#ifdef __cplusplus
extern "C" {
#endif
  void tea5767_delay(uint16_t ms);
  void tea5767_i2c_read(uint8_t addr, uint8_t *data, uint8_t len);
  void tea5767_i2c_write(uint8_t addr, const uint8_t *data, uint8_t len);
  void tea5767_print(const char *msg);

  void tea5767_awake();
  void tea5767_get_default_config(tea5767_config_t *config);
  void tea5767_get_info(tea5767_info_t *info);
  void tea5767_configure(tea5767_config_t *config);
  void tea5767_mute();
  void tea5767_search(tea5767_sud_t sud);
  void tea5767_set_frequency(float mhz);
  void tea5767_standby();
  void tea5767_unmute();

#ifdef __cplusplus
}
#endif

#endif
