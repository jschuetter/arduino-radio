# TEA5767 Arduino Library  

Arduino library for controlling the **TEA5767 FM radio module**.  
Provides a driver and examples for tuning, scanning, mute/unmute, stereo/mono, and standby control.  

---

## Features  
- Tune to a specific frequency
- Automatic scanning (search up/down)
- Mute / Unmute
- Stereo/Mono control
- Standby mode
- Default configuration included

---

## Installation  
1. Download or clone this repository into your Arduino `libraries` folder.  
2. Restart the Arduino IDE.  
3. Open **File → Examples → TEA5767** to try the included demos.  

---

## Usage  

### Basic Example  

```cpp
#include <Wire.h>
#include <TEA5767.h>

// Optional: callback function to display radio status
void radioPrintInfo(const tea5767_info_t *info);

// Optional: callback function to override settings
void radioUpdateConfig(tea5767_config_t *config);

TEA5767 radio(&radioPrintInfo, &radioUpdateConfig); // create instance

void setup() {
  Serial.begin(9600);
  Wire.begin();
  radio.awake();
  radio.setFrequency(100.8);
}

void radioPrintInfo(const tea5767_info_t *info) {
  Serial.print(info->mhz);
  Serial.println(" MHz");
}

void radioUpdateConfig(tea5767_config_t *config) {
  // config->band_limits = TEA5767_BAND_LIMIT_JAPANESE;
  // config->standby = false;
}
```

You can also pass `nullptr` if you don’t want a callbacks.  

---

### Default Configuration  

| Option                        | Default Value | Description                                |
|-------------------------------|---------------|--------------------------------------------|
| `mute`                        | FALSE         | Audio output enabled                       |
| `search_mode`                 | FALSE         | Not in search mode                         |
| `mhz`                         | 87.5 MHz      | Start at lowest band frequency (EU/US)     |
| `search`                      | UP            | Search direction                           |
| `search_stop_level`           | MID           | Stop at mid signal level                   |
| `side_injection`              | LOW           | Use low-side injection                     |
| `mono_to_stereo`              | FALSE         | Stereo ON (default)                        |
| `mute_right`                  | FALSE         | Right channel not muted                    |
| `mute_left`                   | FALSE         | Left channel not muted                     |
| `software_programmable_port1` | FALSE         | Port1 LOW                                  |
| `software_programmable_port2` | FALSE         | Port2 LOW                                  |
| `standby`                     | TRUE          | Start in standby mode                      |
| `band_limits`                 | US/EUROPE     | 87.5–108 MHz band                          |
| `clock_frequency`             | 32.768 kHz    | Default crystal                            |
| `soft_mute`                   | TRUE          | Soft mute enabled                          |
| `high_cut_control`            | TRUE          | High cut enabled                           |
| `stereo_noise_cancelling`     | TRUE          | Stereo noise cancelling enabled            |
| `search_indicator`            | TRUE          | Search indicator on SWPORT1                |
| `de_emphasis_time_constant`   | 50 µs         | De-emphasis for Europe                     |

---

## License  

This project is licensed under the **MIT License**.  

```
MIT License

Copyright (c) 2025 Volodymyr Kumpan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
