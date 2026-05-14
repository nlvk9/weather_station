# ESP32 BME680 Environmental Monitor

An ESP32-based environmental monitoring station that reads temperature and humidity using a BME680 sensor, displays live data on an LCD, triggers multi-mode alarms via LEDs and a buzzer, and uploads readings to ThingSpeak.

---

## Features

- Live temperature (°C and °F) and humidity readings via BME680, displayed on LCD
- Visual and audible alarm system (high temp, low temp, high humidity, combined)
- Automatic data upload to ThingSpeak every 15 seconds

---

## Hardware

| Component | Details |
|---|---|
| Microcontroller | ESP32 |
| Sensor | Adafruit BME680 (I2C) |
| Display | 16x2 LCD (parallel interface) |
| Temp alarm LED | GPIO 27 |
| Humidity alarm LED | GPIO 26 |
| Buzzer | GPIO 35 |

### LCD Wiring (RS, EN, D4, D5, D6, D7) [NON-I2C]

| LCD Pin | ESP32 GPIO |
|---|---|
| RS | 19 |
| EN | 23 |
| D4 | 18 |
| D5 | 5 |
| D6 | 4 |
| D7 | 17 |

### LCD Wiring [I2C]
| LCD Pin | ESP32 GPIO |
|---|---|
| SDA | 21 |
| SCL | 22 |

---

## Dependencies

Install these libraries:

- [Adafruit BME680](https://github.com/adafruit/Adafruit_BME680)
- [Adafruit Unified Sensor](https://github.com/adafruit/Adafruit_Sensor)
- [LiquidCrystal](https://www.arduino.cc/reference/en/libraries/liquidcrystal/) (built-in)
- [ThingSpeak](https://github.com/mathworks/thingspeak-arduino)
- WiFi (built-in with ESP32 board package)

---

## Configuration

### Credentials (secrets)

Rather than hardcoding credentials, create a `secrets.h` file in the same folder as the `.ino` file and add it to `.gitignore`.

**`secrets.h`**
```cpp
#define SECRET_SSID      "your_wifi_ssid"
#define SECRET_PASSWORD  "your_wifi_password"
#define SECRET_TS_KEY    "your_thingspeak_write_api_key"
#define SECRET_TS_CH     000000UL   // your ThingSpeak channel number
```

Then replace the credential variables at the top of the main sketch with:
```cpp
#include "secrets.h"

const char* ssid     = SECRET_SSID;
const char* password = SECRET_PASSWORD;
const char* myWriteAPIKey = SECRET_TS_KEY;
unsigned long myChannelNumber = SECRET_TS_CH;
```

### Alarm Thresholds

Thresholds are defined in `loop()` and can be adjusted to suit your environment:

```cpp
int   humEmergencyValue      = 40;    // humidity % trigger
float tempEmergencyHighValue = 30.0;  // °C upper limit
float tempEmergencyLowValue  = 15.0;  // °C lower limit
```

---

## Alarm Behaviour

| Condition | LED(s) | Buzzer pattern |
|---|---|---|
| High **or** low temp + high humidity | Both alternate | Alternating 4500 Hz / 3000 Hz, 3 cycles |
| High temperature only | Temp LED | Single 3500 Hz pulse |
| Low temperature only | Temp LED | Two short + one long 1500 Hz pulses |
| High humidity only | Humidity LED | Single 500 ms 4000 Hz tone |
| No emergency | Both off | Silent |

---

## ThingSpeak Channel Setup

1. Create a free account at [thingspeak.com](https://thingspeak.com).
2. Create a new channel with at least two fields:
   - **Field 1:** Temperature (°C)
   - **Field 2:** Humidity (%)
3. Copy your **Channel Number** and **Write API Key** into `secrets.h`.

Data is pushed every **14 seconds** (ThingSpeak free tier minimum is 15 s; adjust `14000` in the sketch if you hit rate limits).

---

## Getting Started

1. Clone this repo and open the `.ino` file in your IDE.
2. Install all libraries listed above.
3. Create `secrets.h` with your credentials (see above).
4. Select your ESP32 board under **Tools → Board**.
5. Upload and open the Serial Monitor at **115200 baud**.

---

## .gitignore

Make sure `secrets.h` is never committed:

```
secrets.h
```

---

## License

MIT License. See [LICENSE](LICENSE) for details.
