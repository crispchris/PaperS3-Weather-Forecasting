# 🧭 M5Stack PaperS3 Weather Station (Enhanced)

A self‑contained e‑ink weather dashboard for the **M5Stack PaperS3**.  
Displays current weather, 3‑day forecast, sunrise/sunset, and refreshes on a configurable schedule.  
Includes a **Wi‑Fi configuration web portal** and **icon auto‑scaling**.

---

## ✨ Features

- 🌦 Current weather + 3‑day forecast  
- ☀️ Sunrise / Sunset times  
- 🖼 Icons automatically scaled & centered on screen  
- 🌐 First‑boot captive portal for Wi‑Fi, city, and API key (WiFiManager)  
- ⏰ Automatic NTP time sync  
- 🔋 Deep‑sleep cycles for long battery life  
- 💾 Icons cached in SPIFFS  
- 🧪 Built‑in self‑tests (optional)

---

## 🧩 Hardware / Software Requirements

| Component | Version / Notes |
|------------|----------------|
| **M5Stack PaperS3** | ESP32‑S3‑based e‑ink device |
| **Arduino IDE** | ≥ 2.0 or PlatformIO |
| **ESP32 Boards package** | ≥ 2.0.14 |
| **Libraries** | `M5Unified`, `M5GFX`, `ArduinoJson`, `WiFiManager` |
| **API** | [OpenWeatherMap](https://openweathermap.org/api) (free key) |

---

## ⚙️ Setup Guide

### Install Environment
1. In *File → Preferences*, add  
   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
2. Install **ESP32** in *Tools → Board Manager*.
3. Install libraries:  
   `M5Unified`, `M5GFX`, `ArduinoJson`, `WiFiManager`.

### Board Settings
- **Board:** *M5Stack PaperS3* (or *ESP32‑S3 Dev Module*)  
- **Flash Size:** 16 MB  
- **Partition Scheme:** Default  
- **USB Mode:** CDC and JTAG  
- **Upload Speed:** 921 600 (115 200 if unstable)

### Icons in SPIFFS (optional)

Create a `data/` folder beside the sketch and include BMP icons such as:
icon_01d.bmp
icon_01n.bmp
icon_02d.bmp
...
Upload to SPIFFS using **ESP32 Sketch Data Upload** or PlatformIO’s `uploadfs`.  
Icons will be automatically scaled to the available box on screen.

### First Boot Configuration (Web Portal)
1. Flash the firmware.  
2. On first boot, the device starts an access point `PaperS3Weather`.  
3. Connect via phone or laptop → open `192.168.4.1`.  
4. Enter:
   - Wi‑Fi SSID & Password  
   - City name  
   - OpenWeatherMap API key  
5. Click **Save** — the device will restart and connect automatically.

Settings persist across reboots via WiFiManager’s internal storage.

### Operation Cycle
- Connects to Wi‑Fi → syncs time → fetches current + forecast data.  
- Displays temperature, humidity, conditions, icons, sunrise/sunset.  
- Sleeps for `sleepMinutes` (default = 60).  
- Wakes automatically to refresh.

---

## 🧪 Self‑Tests (optional)

Add to top of the sketch:
```cpp
#define RUN_SELF_TESTS

