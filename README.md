# Smart Desk Info Display (ESP32)

A small desk companion built on an ESP32 that shows the current time and live weather (temperature + humidity) on an SSD1306 OLED display. It connects to Wi-Fi, syncs time via NTP, and pulls weather data from the free Open-Meteo API — no API key required.

This is **v1** of the project, running on a breadboard. A DHT22 sensor, battery power (via buck converter), and a physical on/off switch are planned as follow-up upgrades.

![Prototype on breadboard](photos/first_prototype.jpeg)

## Features

- 📶 Connects to Wi-Fi (ESP32 `WIFI_STA` mode)
- 🕒 Syncs and displays local time via NTP (`configTime`)
- 🌤️ Fetches live temperature and humidity from [Open-Meteo](https://open-meteo.com/) (free, no API key)
- 🖥️ Renders everything on a 128x64 SSD1306 OLED over I2C
- 🔐 Keeps Wi-Fi credentials and location out of source control via a `secrets.h` file

## Hardware Used

| Component | Notes |
|---|---|
| ESP32 dev board | Any standard ESP32 devkit should work |
| SSD1306 OLED display (128x64, I2C) | Connected via `SDA`/`SCL` |
| Breadboard + jumper wires | Current prototype stage |
| USB cable | For power + programming |

> Wiring diagram / breadboard photos go here — see [Photos](#photos) below.

## Libraries Used

Install these via the Arduino Library Manager:

- `Adafruit GFX Library`
- `Adafruit SSD1306`
- `ArduinoJson` (v7 syntax)
- `WiFi` (bundled with the ESP32 board package)
- `HTTPClient` (bundled with the ESP32 board package)

**Board package:** Make sure the ESP32 board package is installed via the Arduino IDE Boards Manager (`esp32` by Espressif Systems).

## How It Works

1. On boot, the ESP32 connects to Wi-Fi and syncs the current time from an NTP server (`pool.ntp.org`).
2. The SSD1306 display initializes and shows a splash screen briefly.
3. In the main loop, the device:
   - Requests current weather data (temperature + humidity) from Open-Meteo based on a fixed latitude/longitude.
   - Reads the current local time.
   - Redraws the display with the latest values.
4. The loop repeats every ~1.5 seconds (weather request delay + loop delay).

Weather data is parsed from the JSON response using `ArduinoJson`, and rendered as simple text using `Adafruit_GFX`.

## Setup

### 1. Clone the repo

```bash
git clone https://github.com/<your-username>/<your-repo>.git
cd <your-repo>
```

### 2. Create your `secrets.h`

This file is **not committed to the repo** (add it to `.gitignore`) and holds your personal Wi-Fi credentials and location. Create a file named `secrets.h` in the same folder as the `.ino` sketch:

```cpp
#pragma once

#define SECRET_SSID "your-wifi-name"
#define SECRET_PASSWORD "your-wifi-password"

#define SECRET_LAT 30.1234
#define SECRET_LON 31.1234

#define SECRET_GMT_OFFSET_SEC 7200      // UTC offset in seconds, e.g. 7200 for UTC+2
#define SECRET_DAYLIGHT_OFFSET_SEC 0    // 3600 if your region observes DST, else 0
```

### 3. Wire up the OLED

Connect the SSD1306 to the ESP32's I2C pins (default `SDA`/`SCL` on most ESP32 boards). The display address used in this project is `0x3C`.

### 4. Flash it

Open the sketch in the Arduino IDE, select your ESP32 board and port, and upload.

## Photos

*(Prototype is still on breadboard — final wiring/enclosure photos will be added once the build is finalized.)*

| | |
|---|---|
| ![Breadboard wiring](photos/breadboard-wiring.jpg) | ![Display close-up](photos/display-closeup.jpg) |
| Breadboard wiring | Display showing time + weather |

> Replace the image paths above with your actual photo files (e.g. add a `photos/` folder to the repo and drop your images in).

## Roadmap

- [ ] Add DHT22 sensor for local temperature/humidity (as a fallback or comparison to the API data)
- [ ] Add a buck converter + battery pack for portable power
- [ ] Add a physical on/off switch
- [ ] Move from breadboard to a soldered/permanent build
- [ ] Possibly add a small enclosure/case (3D printed)

## License

*(Add a license here if you'd like the project to be reusable — e.g. MIT.)*
