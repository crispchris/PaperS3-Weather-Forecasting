#include <M5Unified.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <time.h>
#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager

// ==== CONFIG ====
String city = "London";
String apiKey = "YOUR_OPENWEATHERMAP_API_KEY";
String units = "metric";        // "metric" or "imperial"
bool useDeepSleep = true;
int sleepMinutes = 60;          // refresh interval (minutes)

// ==== NTP CONFIG ====
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 3600;

// ==== GLOBALS ====
HTTPClient http;
float temp, feels_like, humidity;
String weatherDesc, weatherIcon;
time_t sunrise, sunset;
struct ForecastDay { String date; float tmin, tmax; String desc, icon; };
ForecastDay forecast[3];

// ==== UTILITIES ====
void connectWiFi() {
  WiFiManager wm;
  wm.setConfigPortalBlocking(true);
  wm.setTimeout(120);
  WiFiManagerParameter custom_city("city", "City", city.c_str(), 32);
  WiFiManagerParameter custom_api("apikey", "OpenWeatherMap API Key", apiKey.c_str(), 64);
  wm.addParameter(&custom_city);
  wm.addParameter(&custom_api);

  if (!wm.autoConnect("PaperS3Weather")) {
    M5.Lcd.println("WiFiManager timeout, restarting...");
    delay(3000); ESP.restart();
  }
  city = custom_city.getValue();
  apiKey = custom_api.getValue();
  M5.Lcd.printf("Connected: %s\n", WiFi.SSID().c_str());
}

void syncTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) M5.Lcd.println("Time sync failed");
}

bool safeGET(String url, String &out) {
  http.begin(url); http.setTimeout(5000);
  for (int i=0;i<3;i++) {
    int code = http.GET();
    if (code == 200) { out = http.getString(); http.end(); return true; }
    delay(1000);
  }
  http.end(); return false;
}

// ==== WEATHER FETCH ====
bool fetchCurrent() {
  String payload;
  String url = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "&appid=" + apiKey + "&units=" + units;
  if (!safeGET(url, payload)) return false;

  DynamicJsonDocument doc(4096);
  if (deserializeJson(doc, payload)) return false;
  temp = doc["main"]["temp"];
  feels_like = doc["main"]["feels_like"];
  humidity = doc["main"]["humidity"];
  weatherDesc = (const char*)doc["weather"][0]["description"];
  weatherIcon = (const char*)doc["weather"][0]["icon"];
  sunrise = doc["sys"]["sunrise"];
  sunset = doc["sys"]["sunset"];
  return true;
}

bool fetchForecast() {
  String payload;
  String url = "http://api.openweathermap.org/data/2.5/forecast?q=" + city + "&appid=" + apiKey + "&units=" + units;
  if (!safeGET(url, payload)) return false;

  DynamicJsonDocument doc(12288);
  if (deserializeJson(doc, payload)) return false;

  for (int d = 0; d < 3; d++) {
    JsonObject e = doc["list"][d * 8];
    forecast[d].date = String((const char*)e["dt_txt"]).substring(5, 10);
    forecast[d].tmin = e["main"]["temp_min"];
    forecast[d].tmax = e["main"]["temp_max"];
    forecast[d].desc = (const char*)e["weather"][0]["description"];
    forecast[d].icon = (const char*)e["weather"][0]["icon"];
  }
  return true;
}

// ==== ICON HANDLING ====
void drawIcon(String icon, int centerX, int centerY, int boxW, int boxH) {
  String path = "/icon_" + icon + ".bmp";
  File f = SPIFFS.open(path);
  if (!f) {
    M5.Lcd.setCursor(centerX - 20, centerY);
    M5.Lcd.print(icon);
    return;
  }

  // Get image dimensions
  uint32_t sig; f.read((uint8_t*)&sig, 2); f.seek(18);
  int32_t w,h; f.read((uint8_t*)&w,4); f.read((uint8_t*)&h,4);
  f.close();

  float scale = min((float)boxW / w, (float)boxH / h);
  int drawW = w * scale;
  int drawH = h * scale;
  int x = centerX - drawW/2;
  int y = centerY - drawH/2;
  M5.Lcd.drawBmpFile(path.c_str(), x, y, drawW, drawH);
}

// ==== DRAW UI ====
void drawWeather() {
  M5.Lcd.fillScreen(TFT_WHITE);
  M5.Lcd.setTextColor(TFT_BLACK, TFT_WHITE);
  M5.Lcd.setTextSize(2);

  M5.Lcd.setCursor(10, 10);
  M5.Lcd.printf("%s\n", city.c_str());
  M5.Lcd.setCursor(10, 40);
  M5.Lcd.printf("Temp: %.1f°C (%.1f°C)\n", temp, feels_like);
  M5.Lcd.setCursor(10, 70);
  M5.Lcd.printf("Hum: %.0f%%  %s\n", humidity, weatherDesc.c_str());
  drawIcon(weatherIcon, 260, 60, 80, 80);

  // Sunrise / Sunset
  struct tm ts;
  char buf[8];
  M5.Lcd.setTextSize(1);
  localtime_r(&sunrise, &ts); strftime(buf, sizeof(buf), "%H:%M", &ts);
  M5.Lcd.setCursor(10, 110); M5.Lcd.printf("Sunrise: %s\n", buf);
  localtime_r(&sunset, &ts);  strftime(buf, sizeof(buf), "%H:%M", &ts);
  M5.Lcd.setCursor(10, 125); M5.Lcd.printf("Sunset : %s\n", buf);

  M5.Lcd.drawLine(0, 150, 300, 150, TFT_BLACK);
  M5.Lcd.setCursor(10, 160);
  M5.Lcd.setTextSize(2);
  M5.Lcd.println("3-Day Forecast");

  int y = 190;
  for (int i = 0; i < 3; i++) {
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, y);
    M5.Lcd.printf("%s %.1f/%.1f°C %s\n", forecast[i].date.c_str(), forecast[i].tmin, forecast[i].tmax, forecast[i].desc.c_str());
    drawIcon(forecast[i].icon, 260, y, 60, 60);
    y += 40;
  }

  struct tm now; 
  if (getLocalTime(&now)) {
    char tbuf[32]; strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M", &now);
    M5.Lcd.setCursor(10, 310);
    M5.Lcd.printf("Updated: %s", tbuf);
  }
  M5.Lcd.display(); // refresh e-ink
}

// ==== MAIN ====
void setup() {
  auto cfg = M5.config(); M5.begin(cfg);
  SPIFFS.begin(true);
  M5.Lcd.fillScreen(TFT_WHITE);
  M5.Lcd.setTextColor(TFT_BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.println("PaperS3 Weather Portal");

  connectWiFi();
  syncTime();

  bool ok1 = fetchCurrent();
  bool ok2 = fetchForecast();
  if (ok1 && ok2) drawWeather();
  else M5.Lcd.println("Fetch failed.");

  if (useDeepSleep) {
    M5.Lcd.println("Sleeping...");
    M5.Lcd.display();
    WiFi.disconnect(true);
    esp_sleep_enable_timer_wakeup((uint64_t)sleepMinutes * 60ULL * 1000000ULL);
    esp_deep_sleep_start();
  }
}

void loop() { }
