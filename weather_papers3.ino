#include <M5Unified.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <SD.h>

// ==== CONFIG ====
const char* ssid     = "wifi";
const char* password = "pw";
const char* city     = "Valparaiso,CL";
const char* apiKey   = "key";
const char* units    = "metric";
bool useDeepSleep    = true;
int  sleepMinutes    = 60;

// ==== TIME ====
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 3600;

// ==== GLOBALS ====
HTTPClient http;
float temp, feels_like, humidity;
String weatherDesc, weatherIcon;
struct ForecastDay { String date; float tmin, tmax; String desc, icon; };
ForecastDay forecast[3];

// ==== UTILITIES ====
void connectWiFi() {
  Serial.printf("Connecting to WiFi: %s\n", ssid);
  WiFi.begin(ssid, password);
  for (int i=0; i<40 && WiFi.status()!=WL_CONNECTED; i++) { delay(500); Serial.print("."); }
  if (WiFi.status()!=WL_CONNECTED) { Serial.println("WiFi failed, restarting..."); delay(2000); ESP.restart(); }
  Serial.printf("\nConnected. IP: %s\n", WiFi.localIP().toString().c_str());
}

void syncTime() {
  Serial.println("Syncing time...");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm info;
  if (!getLocalTime(&info, 5000)) Serial.println("Time sync failed!");
  else Serial.println("Time sync OK");
}

bool safeGET(String url, String &out) {
  Serial.printf("HTTP GET: %s\n", url.c_str());
  http.begin(url);
  http.setTimeout(8000);
  int code = http.GET();
  if (code == 200) { out = http.getString(); http.end(); return true; }
  Serial.printf("HTTP error: %s\n", http.errorToString(code).c_str());
  http.end(); return false;
}

// ==== WEATHER FETCH ====
bool fetchCurrent() {
  String payload;
  String url = "http://api.openweathermap.org/data/2.5/weather?q=" + String(city) +
               "&appid=" + apiKey + "&units=" + units;
  if (!safeGET(url, payload)) return false;

  DynamicJsonDocument doc(4096);
  if (deserializeJson(doc, payload)) { Serial.println("JSON parse error"); return false; }
  temp        = doc["main"]["temp"];
  feels_like  = doc["main"]["feels_like"];
  humidity    = doc["main"]["humidity"];
  weatherDesc = (const char*)doc["weather"][0]["description"];
  weatherIcon = (const char*)doc["weather"][0]["icon"];
  Serial.printf("Current: %.1f°C hum %.0f%% %s (%s)\n", temp, humidity, weatherDesc.c_str(), weatherIcon.c_str());
  return true;
}

bool fetchForecast() {
  String payload;
  String url = "http://api.openweathermap.org/data/2.5/forecast?q=" + String(city) +
               "&appid=" + apiKey + "&units=" + units;
  if (!safeGET(url, payload)) return false;

  DynamicJsonDocument doc(12288);
  if (deserializeJson(doc, payload)) { Serial.println("Forecast JSON parse error"); return false; }

  for (int d=0; d<3; d++) {
    JsonObject e = doc["list"][d*8];
    if (e.isNull()) continue;
    forecast[d].date = String((const char*)e["dt_txt"]).substring(5,10);
    forecast[d].tmin = e["main"]["temp_min"];
    forecast[d].tmax = e["main"]["temp_max"];
    forecast[d].desc = (const char*)e["weather"][0]["description"];
    forecast[d].icon = (const char*)e["weather"][0]["icon"];
  }
  return true;
}

// ==== ICONS ====
String emoji(String code) {
  if (code.startsWith("01")) return "☀";
  if (code.startsWith("02")) return "🌤";
  if (code.startsWith("03") || code.startsWith("04")) return "☁";
  if (code.startsWith("09") || code.startsWith("10")) return "🌧";
  if (code.startsWith("11")) return "⚡";
  if (code.startsWith("13")) return "❄";
  if (code.startsWith("50")) return "🌫";
  return "?";
}

void resetScreen() {
  Serial.println("Drawing to display...");
  M5.Display.wakeup();
  M5.Display.setEpdMode(epd_mode_t::epd_fast);
  M5.Display.setRotation(1);
  M5.Display.fillScreen(TFT_WHITE);
  M5.Display.display();
  M5.Display.waitDisplay();
}

// ==== DRAW ====
void drawWeather() {
  Serial.println("Drawing to display...");
  resetScreen();
  resetScreen();
  resetScreen();
  M5.Display.wakeup();
  M5.Display.setEpdMode(epd_mode_t::epd_fast);
  M5.Display.setRotation(1);
  M5.Display.fillScreen(TFT_WHITE);
  M5.Display.setTextColor(TFT_BLACK, TFT_WHITE);

  for (int i = 0; i < 3; i++) {
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.setTextSize(4);
    M5.Display.setCursor(10, 10);
    M5.Display.printf("%s", city);

    String e = emoji(weatherIcon);
    M5.Display.setTextSize(8);
    M5.Display.setCursor(10, 60);
    M5.Display.printf("%s", e.c_str());
    M5.Display.setTextSize(8);
    M5.Display.setCursor(140, 80);
    M5.Display.printf("%.1f°C", temp);

    M5.Display.setTextSize(4);
    M5.Display.setCursor(10, 200);
    M5.Display.printf("%s | Feels %.1f°C | Hum %.0f%%", weatherDesc.c_str(), feels_like, humidity);

    int y = 230;
    M5.Display.drawLine(0, y, 320, y, TFT_BLACK);
    y += 10;
    M5.Display.setTextSize(4);
    for (int i=0;i<3;i++) {
      String em = emoji(forecast[i].icon);
      M5.Display.setCursor(10, y);
      M5.Display.printf("%s %s %.0f/%.0f°C", forecast[i].date.c_str(), em.c_str(), forecast[i].tmax, forecast[i].tmin);
      y += 25;
    }

    struct tm now;
    if (getLocalTime(&now)) {
      char buf[32];
      strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", &now);
      M5.Display.setTextSize(2);
      M5.Display.setCursor(10, y+10);
      M5.Display.printf("Updated: %s", buf);
    }

    M5.Display.display();
    M5.Display.waitDisplay();
  }
  delay(2000);
  Serial.println("Display updated.");
}

// ==== MAIN ====
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== PaperS3 Weather ===");

  auto cfg = M5.config();
  cfg.output_power = true;
  M5.begin(cfg);  
  M5.Power.begin();

  M5.Display.wakeup();
  M5.Display.setEpdMode(epd_mode_t::epd_quality);
  M5.Display.fillScreen(TFT_WHITE);
  M5.Display.setTextColor(TFT_BLACK);
  M5.Display.setTextSize(3);
  M5.Display.setCursor(20, 100);
  M5.Display.println("Connecting WiFi...");
  M5.Display.display();

  connectWiFi();
  syncTime();

  bool ok1 = fetchCurrent();
  bool ok2 = fetchForecast();
  
  if (ok1 && ok2) drawWeather();
  else {
    M5.Display.fillScreen(TFT_WHITE);
    M5.Display.setTextColor(TFT_BLACK);
    M5.Display.setTextSize(3);
    M5.Display.setCursor(20, 100);
    M5.Display.println("Fetch failed");
    M5.Display.display();
  }

  if (useDeepSleep) {
    WiFi.disconnect(true);
    esp_sleep_enable_timer_wakeup((uint64_t)sleepMinutes * 60ULL * 1000000ULL);
    esp_deep_sleep_start();
  }
}

void loop() {}
