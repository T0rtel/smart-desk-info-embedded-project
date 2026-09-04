# ESP32 Smart Desk Info Display

A small ESP32-based desk information display that connects to Wi-Fi, retrieves live weather data from the Open-Meteo API, synchronizes the current time using NTP, and displays the information on a 0.96" 128×64 OLED screen.

This is the **first version of the project**. The current prototype retrieves temperature and humidity from the internet. A DHT22 sensor will be added in a future version to measure the actual conditions around the device.

---

## 📸 Project Preview

> **V1 Prototype — Breadboard**

![ESP32 Smart Desk Info Prototype](images/v1-prototype.jpg)

*Current prototype running on an ESP32 with a 128×64 OLED display.*

---

## 🎥 Demo

<!-- Add a short demo video/GIF here later -->

[▶️ Demo video](YOUR_VIDEO_LINK_HERE)

---

## ✨ Features

### Current Version (V1)

- ESP32-based
- 0.96" 128×64 OLED display
- I2C communication with the OLED
- Wi-Fi connectivity
- Live weather data from Open-Meteo
- Temperature display
- Humidity display
- Current time using NTP
- JSON API response parsing
- Configurable location
- Credentials separated into a `secrets.h` file

### Planned

- [ ] Add DHT22 temperature/humidity sensor
- [ ] Display locally measured temperature and humidity
- [ ] Add battery power
- [ ] Add buck converter / appropriate power regulation
- [ ] Add physical power switch
- [ ] Design a cleaner enclosure
- [ ] Improve the display UI
- [ ] Improve Wi-Fi connection handling
- [ ] Add more information/features

---

# 🧰 Hardware

### Current Prototype

| Component | Purpose |
|---|---|
| ESP32 Development Board | Main microcontroller |
| 0.96" 128×64 OLED | Information display |
| Breadboard | Prototyping |
| Jumper wires | Connections |

### Planned Hardware

| Component | Purpose |
|---|---|
| DHT22 | Local temperature & humidity measurement |
| Battery | Portable power |
| Buck converter / voltage regulator | Power regulation |
| Switch | Physical power control |
| Enclosure | Final housing |

---

# 💻 Software

- C++
- Arduino framework
- Arduino IDE
- Adafruit GFX
- Adafruit SSD1306
- ArduinoJson
- WiFi
- HTTPClient
- NTP / `time.h`

### External API

Weather data is provided by:

**Open-Meteo**

https://open-meteo.com/

Open-Meteo provides weather data without requiring an API key, which made it a convenient choice for this project.

---

# 🔌 How It Works

The project can be broken down into several parts:

```text
                  Internet
                     │
                     │ Wi-Fi
                     ▼
                  ESP32
                /       \
               /         \
              ▼           ▼
        Open-Meteo       NTP
          Weather         Time
              │           │
              └─────┬─────┘
                    ▼
               ESP32 Data
                    │
                    ▼
              OLED Display
