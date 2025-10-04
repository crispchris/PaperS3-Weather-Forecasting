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
   `https://static-cdn.m5stack.com/resource/arduino/package_m5stack_index.json`
3. Install **ESP32** in *Tools → Board Manager*.
4. Install libraries:  
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
```
On boot, the device validates:
- Wi‑Fi connectivity  
- JSON parser integrity  

Messages appear on both LCD and serial console.

---

## 🩹 Troubleshooting

| Symptom | Likely Cause | Fix |
|----------|--------------|-----|
| Blank screen | E‑ink not refreshed | Ensure `M5.Lcd.display()` executes after drawing |
| “Fetch failed.” | API key invalid or Wi‑Fi error | Re‑run WiFiManager portal |
| No icons | SPIFFS empty or wrong filenames | Upload `/data` folder again |
| Wrong time | NTP failure / offset | Verify `ntpServer` and offset |
| Frequent resets | Power issue | Use 5 V ≥ 1 A USB supply |

---

## 🧮 Configuration Variables

| Variable | Purpose | Default |
|-----------|----------|----------|
| `city` | Weather location name | `"London"` |
| `apiKey` | OpenWeatherMap API key | `"YOUR_OPENWEATHERMAP_API_KEY"` |
| `units` | `"metric"` or `"imperial"` | `"metric"` |
| `sleepMinutes` | Refresh interval (min) | `60` |
| `useDeepSleep` | Enable power‑saving mode | `true` |

---

## 🧰 Testing on Desktop (optional)

You can isolate the JSON parsing logic on a host system:

```bash
g++ tests/test_json.cpp -o test && ./test
```

`tests/test_json.cpp` can reuse the same parsing functions with mocked API payloads to validate structures.

---

## 🌅 Displayed Information

| Section | Data Source | Example |
|----------|-------------|----------|
| Current weather | `/data/2.5/weather` | “Temp 21.3 °C Feels 19.8 °C Clear Sky” |
| Forecast | `/data/2.5/forecast` | 3 entries ≈ 1 per day |
| Sunrise / Sunset | `sys.sunrise` / `sys.sunset` | “Sunrise 07:12  Sunset 18:34” |
| Time stamp | NTP (localtime) | “Updated 2025‑10‑04 10:00” |

---

## 🔧 Future Ideas

- 7‑day forecast view  
- On‑device settings menu with buttons  
- Low‑power Wi‑Fi reconnect optimization

---

## 📜 License

MIT © 2025 — You.

---

## 📸 Screenshot

![PaperS3 Weather Example](docs/example_screen.jpg)
```


