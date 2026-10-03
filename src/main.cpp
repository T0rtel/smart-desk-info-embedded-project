#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <time.h>

#include "secrets.h"


#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT  64
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


#define DHT_PIN  15
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

// Cities struct for the scrolling title bar 
struct City {
  const char* name;
  float lat;
  float lon;
  float temp;
  int   hum; 
};

City cities[] = {
  {"London",   51.5074f,  -0.1278f, NAN, -1},
  {"Dubai",    25.2048f,  55.2708f, NAN, -1},
  {"New York", 40.7128f, -74.0060f, NAN, -1},
  {"Tokyo",    35.6762f, 139.6503f, NAN, -1},
};
const int CITY_COUNT = sizeof(cities) / sizeof(cities[0]);

// Settings
const unsigned long WEATHER_INTERVAL_MS = 10UL * 60UL * 1000UL; // refresh every 10 min
const unsigned long WEATHER_RETRY_MS    = 30UL * 1000UL;        // retry after a failure
const unsigned long DHT_UPDATE_DEBOUNCE = 2000;              // DHT11 max rate is ~1 Hz
const unsigned long FRAME_UPDATE_DEBOUNCE  = 40;                   // ~25 px/s scroll speed (1 px per frame)
const int TICKER_GAP_PX = 36;                                   // blank space between ticker repeats

const char* ssid     = SECRET_SSID;
const char* password = SECRET_PASSWORD;
const float latitude  = SECRET_LAT;
const float longitude = SECRET_LON;
const long gmtOffset_sec      = SECRET_GMT_OFFSET_SEC;
const int  daylightOffset_sec = SECRET_DAYLIGHT_OFFSET_SEC;
const char* ntpServer = "pool.ntp.org";

// State 
float outTemp = NAN;   
int   outHum  = -1;
float roomTemp = NAN;  
float roomHum  = NAN;

String tickerText = "Smart Desk Info";
int scrollX = 0;

unsigned long lastDht = 0, lastFrame = 0, nextWeather = 0, nextReconnect = 0;

// Weather 
void buildTicker();

// One request for home + every city (Open-Meteo accepts comma-separated coordinates).
bool fetchWeather() {
  String lats = String(latitude, 4);
  String lons = String(longitude, 4);
  for (int i = 0; i < CITY_COUNT; i++) {
    lats += "," + String(cities[i].lat, 4);
    lons += "," + String(cities[i].lon, 4);
  }

  String url = "https://api.open-meteo.com/v1/forecast?latitude=" + lats +
               "&longitude=" + lons +
               "&current=temperature_2m,relative_humidity_2m";

  HTTPClient http;
  http.setTimeout(8000);
  http.begin(url);
  int code = http.GET();
  if (code != 200) {
    Serial.printf("HTTP GET failed: %d\n", code);
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload);
  if (err) {
    Serial.print("JSON parse failed: ");
    Serial.println(err.c_str());
    return false;
  }

  // if its Several locations thats because its a JSON array.
  bool ismulti = doc.is<JsonArray>();

  JsonVariant home;
  if (ismulti) home = doc[0];
  else       home = doc.as<JsonVariant>();

  outTemp = home["current"]["temperature_2m"] | NAN;
  outHum  = home["current"]["relative_humidity_2m"] | -1;

  if (ismulti) {
    for (int i = 0; i < CITY_COUNT; i++) {
      JsonVariant c = doc[i + 1];
      cities[i].temp = c["current"]["temperature_2m"] | NAN;
      cities[i].hum  = c["current"]["relative_humidity_2m"] | -1;
    }
  }

  buildTicker();
  return true;
}
// the final scrolling text
void buildTicker() {
  tickerText = "";
  for (int i = 0; i < CITY_COUNT; i++) {
    if (isnan(cities[i].temp)) continue;
    char buf[40];
    snprintf(buf, sizeof(buf), "%s %.0fC %d%%", cities[i].name, cities[i].temp, cities[i].hum);
    if (tickerText.length()) tickerText += "  *  ";
    tickerText += buf;
  }
  if (tickerText.length() == 0) tickerText = "Smart Desk Info";
}

// Drawing helpers
// Big number + small degree sign + "C"
void drawTemp(int x, int y, float t, int decimals) {
  char buf[8];
  if (isnan(t)) strcpy(buf, "--");
  else snprintf(buf, sizeof(buf), "%.*f", decimals, t);

  display.setTextSize(2);
  display.setCursor(x, y);
  display.print(buf);

  int endX = x + strlen(buf) * 12;
  display.drawCircle(endX + 3, y + 2, 2, SSD1306_WHITE); // degree sign
  display.setTextSize(1);
  display.setCursor(endX + 8, y);
  display.print("C");
}

void drawHumidity(int x, int y, float h) {
  display.setTextSize(1);
  display.setCursor(x, y);
  display.print("Hum ");
  if (isnan(h) || h < 0) display.print("--");
  else display.print((int)round(h));
  display.print("%");
}

void renderFrame() {
  display.clearDisplay();
  display.setTextSize(1);

  // ---- Scrolling title bar: fills the whole yellow strip (y 0-15) ----
  display.fillRect(0, 0, SCREEN_WIDTH, 16, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  int w = tickerText.length() * 6 + TICKER_GAP_PX;
  display.setCursor(-scrollX, 4);
  display.print(tickerText);
  display.setCursor(w - scrollX, 4);   // second copy for seamless wrap
  display.print(tickerText);
  scrollX = (scrollX + 1) % w;

  // ---- Body: everything below is in the blue zone (y 16-63) ----
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 18);
  display.print("OUTSIDE");
  display.setCursor(70, 18);
  display.print("ROOM");
  display.drawFastVLine(64, 18, 34, SSD1306_WHITE);

  drawTemp(0, 28, outTemp, 1);     // internet temp, 1 decimal
  drawTemp(70, 28, roomTemp, 0);   // DHT11 is whole-degree only

  drawHumidity(0, 45, outHum);
  drawHumidity(70, 45, roomHum);

  // ---- Footer: date + time ----
  display.drawFastHLine(0, 54, SCREEN_WIDTH, SSD1306_WHITE);

  char dateBuf[16] = "--";
  char timeBuf[12] = "--:--";
  struct tm ti;
  if (getLocalTime(&ti, 0)) {
    strftime(dateBuf, sizeof(dateBuf), "%a %d %b", &ti);
    strftime(timeBuf, sizeof(timeBuf), "%I:%M %p", &ti);
  }
  display.setCursor(0, 56);
  display.print(dateBuf);
  display.setCursor(SCREEN_WIDTH - strlen(timeBuf) * 6, 56);
  display.print(timeBuf);

  display.display();
}

void showMessage(const char* msg) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 28);
  display.print(msg);
  display.display();
}

// ---------------- Arduino ----------------
void setup() {
  Serial.begin(9600);
  dht.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.setTextWrap(false);   // needed so the ticker can slide off-screen
  showMessage("Connecting WiFi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  showMessage("Getting data...");
}

void loop() {
  unsigned long now = millis();

  // Read the DHT11
  if (now - lastDht >= DHT_UPDATE_DEBOUNCE) {
    lastDht = now;
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t)) roomTemp = t;
    if (!isnan(h)) roomHum = h;
  }

  // Refresh internet weather (home + ticker cities)
  if ((long)(now - nextWeather) >= 0) {
    if (WiFi.status() == WL_CONNECTED) {
      bool success = fetchWeather();
      nextWeather = millis() + (success ? WEATHER_INTERVAL_MS : WEATHER_RETRY_MS);
    } else {
      nextWeather = millis() + 5000;
    }
  }

  // Reconnect WiFi if it dropped
  if (WiFi.status() != WL_CONNECTED && (long)(now - nextReconnect) >= 0) {
    WiFi.reconnect();
    nextReconnect = now + 10000;
  }

  // Draw a frame (also advances the ticker)
  if (now - lastFrame >= FRAME_UPDATE_DEBOUNCE) {
    lastFrame = now;
    renderFrame();
  }
}