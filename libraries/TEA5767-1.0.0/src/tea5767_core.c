#include "tea5767_core.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

static uint8_t write_conf_buf[5];
static uint8_t read_conf_buf[5];

// if MUTE = true then L and R audio are muted;
// if MUTE = false then L and R audio are not muted
// 1000 0000
static void set_mute_bits(bool mute) {
  if (mute) {
    write_conf_buf[0] |= 0x80;
  } else {
    write_conf_buf[0] &= ~0x80;
  }
}
static bool get_mute(void) {
  return (write_conf_buf[0] >> 7) & 0x01;
}

// Search Mode:
// if SM = true then in search mode;
// if SM = false then not in search mode
// 0100 0000
static void set_search_mode_bits(bool sm) {
  if (sm) {
    write_conf_buf[0] |= 0x40;
  } else {
    write_conf_buf[0] &= ~0x40;
  }
}

// setting of synthesizer programmable counter for search or preset
// 0011 1111
// 1111 1111
static void set_pll_bits(uint16_t pll) {
  write_conf_buf[0] &= ~0x3F;
  write_conf_buf[0] |= (pll >> 8) & 0x3F;
  write_conf_buf[1] = pll & 0xFF;
}

// Search Up/Down:
// if SUD = TEA5767_SEARCH_UP then search up;
// if SUD = TEA5767_SEARCH_DOWN then search down
// 1000 0000
static void set_search_up_down_bits(tea5767_sud_t sud) {
  if (sud == TEA5767_SEARCH_UP) {
    write_conf_buf[2] |= 0x80;
  } else {
    write_conf_buf[2] &= ~0x80;
  }
}

// Search Stop Level:
// 0 0 not allowed in search mode
// 0 1 low; level ADC output = 5, 0010 0000
// 1 0 mid; level ADC output = 7, 0100 0000
// 1 1 high; level ADC output = 10, 0110 0000
static void set_search_stop_level_bits(tea5767_ssl_t ssl) {
  write_conf_buf[2] &= ~0x60;
  if (ssl == TEA5767_SEARCH_STOP_LEVEL_LOW) {
    write_conf_buf[2] |= 0x20;
  } else if (ssl == TEA5767_SEARCH_STOP_LEVEL_MID) {
    write_conf_buf[2] |= 0x40;
  } else if (ssl == TEA5767_SEARCH_STOP_LEVEL_HIGH) {
    write_conf_buf[2] |= 0x60;
  }
}

// HIGH/LOW Side Injection:
// if HLSI = TEA5767_SIDE_INJECTION_HIGH then HIGH side LO injection;
// if HLSI = TEA5767_SIDE_INJECTION_LOW then LOW side LO injection
// 0001 0000
static void set_high_low_side_injection_bits(tea5767_hlsi_t hlsi) {
  if (hlsi == TEA5767_SIDE_INJECTION_HIGH) {
    write_conf_buf[2] |= 0x10;
  } else {
    write_conf_buf[2] &= ~0x10;
  }
}
static tea5767_hlsi_t get_high_low_side_injection(void) {
  return (tea5767_hlsi_t)((write_conf_buf[2] >> 4) & 0x01);
}

// Mono to Stereo:
// if MS = true then forced mono;
// if MS = false then stereo ON
// 0000 1000
static void set_mono_to_stereo_bits(bool ms) {
  if (ms) {
    write_conf_buf[2] |= 0x08;
  } else {
    write_conf_buf[2] &= ~0x08;
  }
}

// Mute Right:
// if MR = true then the right audio channel is muted and forced mono;
// if MR = false then the right audio channel is not muted
// 0000 0100
static void set_mute_right_bits(bool mr) {
  if (mr) {
    write_conf_buf[2] |= 0x04;
  } else {
    write_conf_buf[2] &= ~0x04;
  }
}

// Mute Left:
// if ML = true then the left audio channel is muted and forced mono;
// if ML = false then the left audio channel is not muted
// 0000 0010
static void set_mute_left_bits(bool ml) {
  if (ml) {
    write_conf_buf[2] |= 0x02;
  } else {
    write_conf_buf[2] &= ~0x02;
  }
}

// Software programmable port 1:
// if SWP1 = true then port 1 is HIGH;
// if SWP1 = false then port 1 is LOW
// 0000 0001
static void set_software_programmable_port1_bits(bool swp1) {
  if (swp1) {
    write_conf_buf[2] |= 0x01;
  } else {
    write_conf_buf[2] &= ~0x01;
  }
}

// Software programmable port 2:
// if SWP2 = true then port 2 is HIGH;
// if SWP2 = false then port 2 is LOW
// 1000 0000
static void set_software_programmable_port2_bits(bool swp2) {
  if (swp2) {
    write_conf_buf[3] |= 0x80;
  } else {
    write_conf_buf[3] &= ~0x80;
  }
}

// Standby:
// if STBY = true then in standby mode;
// if STBY = false then not in standby mode
// 0100 0000
static void set_standby_bits(bool stby) {
  if (stby) {
    write_conf_buf[3] |= 0x40;
  } else {
    write_conf_buf[3] &= ~0x40;
  }
}
static bool get_standby(void) {
  return (write_conf_buf[3] >> 6) & 0x01;
}

// Band Limits:
// if BL = TEA5767_FM_BAND_JAPANESE then Japanese FM band;
// if BL = TEA5767_FM_BAND_US_EUROPE then US/Europe FM band
// 0010 0000
static void set_band_limits_bits(tea5767_bl_t bl) {
  if (bl == TEA5767_BAND_LIMIT_JAPANESE) {
    write_conf_buf[3] |= 0x20;
  } else {
    write_conf_buf[3] &= ~0x20;
  }
}
static tea5767_bl_t get_band_limits() {
  return (tea5767_bl_t)((write_conf_buf[3] >> 5) & 0x01);
}

// if XTAL = TEA5767_CLOCK_32768HZ then fxtal = 32.768 kHz;
// if XTAL = TEA5767_CLOCK_13MHZ then fxtal = 13 MHz
// 0001 0000
static void set_xtal_bits(tea5767_clock_t xtal) {
  if (xtal == TEA5767_CLOCK_32768HZ) {
    write_conf_buf[3] |= 0x10;
  } else {
    write_conf_buf[3] &= ~0x10;
  }
}
static uint32_t get_clock(void) {
  bool xtal = ((write_conf_buf[3] >> 4) & 0x01);    // 0001 0000
  bool pllref = ((write_conf_buf[4] >> 7) & 0x01);  // 1000 0000

  if (!xtal && !pllref) {
    return 13000000;
  } else if (xtal && !pllref) {
    return 32768;
  } else if (!xtal && pllref) {
    return 6500000;
  } else {
    return 0;
  }
}

// Soft MUTE:
// if SMUTE = true then soft mute is ON;
// if SMUTE = false then soft mute is OFF
// 0000 1000
static void set_soft_mute_bits(bool smute) {
  if (smute) {
    write_conf_buf[3] |= 0x08;
  } else {
    write_conf_buf[3] &= ~0x08;
  }
}

// High Cut Control:
// if HCC = true then high cut control is ON;
// if HCC = false then high cut control is OFF
// 0000 0100
static void set_high_cut_control_bits(bool hcc) {
  if (hcc) {
    write_conf_buf[3] |= 0x04;
  } else {
    write_conf_buf[3] &= ~0x04;
  }
}

// Stereo Noise Cancelling:
// if SNC = true then stereo noise cancelling is ON;
// if SNC = false then stereo noise cancelling is OFF
// 0000 0010
static void set_stereo_noise_cancelling_bits(bool snc) {
  if (snc) {
    write_conf_buf[3] |= 0x02;
  } else {
    write_conf_buf[3] &= ~0x02;
  }
}

// Search Indicator:
// if SI = true then pin SWPORT1 is output for the ready flag;
// if SI = false then pin SWPORT1 is software programmable port 1
// 0000 0001
static void set_search_indicator_bits(bool si) {
  if (si) {
    write_conf_buf[3] |= 0x01;
  } else {
    write_conf_buf[3] &= ~0x01;
  }
}

// if PLLREF = TEA5767_CLOCK_6500KHZ then the 6.5 MHz reference frequency for the PLL is enabled;
// if PLLREF != TEA5767_CLOCK_6500KHZ then the 6.5 MHz reference frequency for the PLL is disabled
// 1000 0000
static void set_pllref_bits(tea5767_clock_t pllref) {
  if (pllref == TEA5767_CLOCK_6500KHZ) {
    write_conf_buf[4] |= 0x80;
  } else {
    write_conf_buf[4] &= ~0x80;
  }
}

// if DTC = TEA5767_DE_EMPHASIS_TIME_CONSTANT_75US then the de-emphasis time constant is 75 µs;
// if DTC = TEA5767_DE_EMPHASIS_TIME_CONSTANT_50US then the de-emphasis time constant is 50 µs
// 0100 0000
static void set_dtc_bits(tea5767_dtc_t dtc) {
  if (dtc == TEA5767_DE_EMPHASIS_TIME_CONSTANT_75US) {
    write_conf_buf[4] |= 0x40;
  } else {
    write_conf_buf[4] &= ~0x40;
  }
}

// Bits: 5 to 0 not used; position is don’t care

// Ready Flag:
// if RF = true then a station has been found or the band limit has been reached;
// if RF = false then no station has been found
// 1000 0000
static bool get_ready_flag(void) {
  return (read_conf_buf[0] >> 7) & 0x01;
}

// Band Limit Flag:
// if BLF = true then the band limit has been reached;
// if BLF = false then the band limit has not been reached
// 0100 0000
static bool get_band_limit_flag(void) {
  return (read_conf_buf[0] >> 6) & 0x01;
}

// setting of synthesizer programmable counter after search or preset
// 0011 1111
static uint16_t get_pll(void) {
  return ((read_conf_buf[0] & 0x3F) << 8) | read_conf_buf[1];
}

// Stereo indication:
// if STEREO = true then stereo reception;
// if STEREO = false then mono reception
// 1000 0000
static bool get_stereo_indication(void) {
  return (read_conf_buf[2] >> 7) & 0x01;
}

// IF counter result
// 0111 1111
static uint8_t get_if_counter_result(void) {
  return read_conf_buf[2] & 0x7F;
}

// level ADC output
// 1111 0000
static uint8_t get_level_adc_output(void) {
  return (read_conf_buf[3] >> 4) & 0x0F;
}

// Chip Identification: these bits have to be set to logic 0
// 0000 1110
static uint8_t get_chip_identification(void) {
  return (read_conf_buf[3] >> 1) & 0x07;
}

// Bit 0: this bit is internally set to logic 0

// Bits 7 to 0: reserved for future extensions; these bits are internally set to logic 0

static void log_buf_bin(const uint8_t *buf) {
  char line[9];
  for (uint8_t i = 0; i < 5; i++) {
    for (int bit = 7; bit >= 0; bit--) {
      line[7 - bit] = (buf[i] & (1 << bit)) ? '1' : '0';
    }
    line[8] = '\0';
    tea5767_print(line);
  }
}

static uint16_t to_pll(float mhz) {
  uint32_t hz = mhz * 1000000;
  tea5767_hlsi_t hlsi = get_high_low_side_injection();
  uint32_t lo = hlsi == TEA5767_SIDE_INJECTION_HIGH
                  ? hz + TEA5767_IF_OFFSET_HZ
                  : hz - TEA5767_IF_OFFSET_HZ;
  uint32_t clock = get_clock();
  uint16_t pll = lo * 4 / clock;

  return pll;
}

static float to_mhz(uint16_t pll) {
  uint32_t clock = get_clock();
  uint32_t lo = (uint32_t)pll * clock / 4;
  tea5767_hlsi_t hlsi = get_high_low_side_injection();
  uint32_t hz = hlsi == TEA5767_SIDE_INJECTION_HIGH
                  ? lo - TEA5767_IF_OFFSET_HZ
                  : lo + TEA5767_IF_OFFSET_HZ;
  float mhz = (float)hz / 1000000;

  return mhz;
}

void tea5767_awake() {
  set_standby_bits(false);

  tea5767_i2c_write(TEA5767_IC_ADDRESS, write_conf_buf, 5);
}

void tea5767_get_default_config(tea5767_config_t *config) {
  if (!config)
    return;

  config->mute = false;
  config->search_mode = false;
  config->mhz = TEA5767_US_EUROPE_BAND_MIN;
  config->search = TEA5767_SEARCH_UP;
  config->search_stop_level = TEA5767_SEARCH_STOP_LEVEL_MID;
  config->side_injection = TEA5767_SIDE_INJECTION_LOW;
  config->mono_to_stereo = false;
  config->mute_right = false;
  config->mute_left = false;
  config->software_programmable_port1 = false;
  config->software_programmable_port2 = false;
  config->standby = true;
  config->band_limits = TEA5767_BAND_LIMIT_US_EUROPE;
  config->clock_frequency = TEA5767_CLOCK_32768HZ;
  config->soft_mute = true;
  config->high_cut_control = true;
  config->stereo_noise_cancelling = true;
  config->search_indicator = true;
  config->de_emphasis_time_constant = TEA5767_DE_EMPHASIS_TIME_CONSTANT_50US;
}

void tea5767_get_info(tea5767_info_t *info) {
  if (!info) {
    return;
  }
  memset(info, 0, sizeof(*info));

  tea5767_delay(200);
  tea5767_i2c_read(TEA5767_IC_ADDRESS, read_conf_buf, 5);

  info->muted = get_mute();
  info->standby = get_standby();
  info->ready = get_ready_flag();
  info->band_limit_reached = get_band_limit_flag();
  uint16_t pll = get_pll();
  info->mhz = to_mhz(pll);
  info->stereo = get_stereo_indication();
  uint8_t ifc = get_if_counter_result();
  info->tune = ifc - TEA5767_MAX_IF_COUNTER / 2;
  uint8_t adc = get_level_adc_output();
  info->signal_level = adc * 100 / TEA5767_MAX_ADC;
  info->chip_id = get_chip_identification();
}

void tea5767_configure(tea5767_config_t *config) {
  tea5767_config_t conf;
  if (!config) {
    tea5767_get_default_config(&conf);
  } else {
    conf = *config;
  }

  set_high_low_side_injection_bits(conf.side_injection);
  set_xtal_bits(conf.clock_frequency);
  set_pllref_bits(conf.clock_frequency);

  set_mute_bits(conf.mute);
  set_search_mode_bits(conf.search_mode);
  uint16_t pll = to_pll(conf.mhz);
  set_pll_bits(pll);

  set_search_up_down_bits(conf.search);
  set_search_stop_level_bits(conf.search_stop_level);
  set_mono_to_stereo_bits(conf.mono_to_stereo);
  set_mute_right_bits(conf.mute_right);
  set_mute_left_bits(conf.mute_left);
  set_software_programmable_port1_bits(conf.software_programmable_port1);

  set_software_programmable_port2_bits(conf.software_programmable_port2);
  set_standby_bits(conf.standby);
  set_band_limits_bits(conf.band_limits);
  set_soft_mute_bits(conf.soft_mute);
  set_high_cut_control_bits(conf.high_cut_control);
  set_stereo_noise_cancelling_bits(conf.stereo_noise_cancelling);
  set_search_indicator_bits(conf.search_indicator);

  set_dtc_bits(conf.de_emphasis_time_constant);
}

void tea5767_mute() {
  set_mute_bits(true);

  tea5767_i2c_write(TEA5767_IC_ADDRESS, write_conf_buf, 5);
}

void tea5767_search(tea5767_sud_t sud) {
  tea5767_i2c_read(TEA5767_IC_ADDRESS, read_conf_buf, 5);
  uint16_t pll = get_pll();
  uint16_t searched_pll;
  for (int i = 0; i < 60; i++) {
    set_search_mode_bits(true);
    set_search_up_down_bits(sud);
    tea5767_i2c_write(TEA5767_IC_ADDRESS, write_conf_buf, 5);
    tea5767_delay(200);

    bool ready_flag = false;
    bool band_limit_reached = false;
    while (!ready_flag) {
      tea5767_i2c_read(TEA5767_IC_ADDRESS, read_conf_buf, 5);
      ready_flag = get_ready_flag();
      band_limit_reached = get_band_limit_flag();

      tea5767_delay(200);
    }

    set_search_mode_bits(false);
    searched_pll = get_pll();

    if (band_limit_reached) {
      break;
    }

    if (sud == TEA5767_SEARCH_UP) {
      if (searched_pll > pll) {
        break;
      }
      pll = pll + 1;
    } else {
      if (searched_pll < pll) {
        break;
      }
      pll = pll - 1;
    }
    set_pll_bits(pll);
  }
  set_pll_bits(searched_pll);
}

void tea5767_set_frequency(float mhz) {
  tea5767_bl_t bl = get_band_limits();
  float fm_band_min;
  float fm_band_max;
  if (bl == TEA5767_BAND_LIMIT_US_EUROPE) {
    fm_band_min = TEA5767_US_EUROPE_BAND_MIN;
    fm_band_max = TEA5767_US_EUROPE_BAND_MAX;
  } else {
    fm_band_min = TEA5767_JAPANESE_BAND_MIN;
    fm_band_max = TEA5767_JAPANESE_BAND_MAX;
  }
  if (mhz < fm_band_min || mhz > fm_band_max) {
    return;
  }

  uint16_t pll = to_pll(mhz);
  set_pll_bits(pll);

  tea5767_i2c_write(TEA5767_IC_ADDRESS, write_conf_buf, 5);
}

void tea5767_standby() {
  set_standby_bits(true);

  tea5767_i2c_write(TEA5767_IC_ADDRESS, write_conf_buf, 5);
}

void tea5767_unmute() {
  set_mute_bits(false);

  tea5767_i2c_write(TEA5767_IC_ADDRESS, write_conf_buf, 5);
}
