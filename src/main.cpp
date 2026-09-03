#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 


#define OLED_RESET     -1 
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// const char* ssid = "OrangeDSL-Hassan Youssef";
// const char* password = "kzx-7hara";

// const float latitude = 30.103905;
// const float longitude = 31.330242;

#include "secrets.h"

const char* ssid = SECRET_SSID;
const char* password = SECRET_PASSWORD;

const float latitude = SECRET_LAT;
const float longitude = SECRET_LON;

const long gmtOffset_sec = SECRET_GMT_OFFSET_SEC;
const int daylightOffset_sec = SECRET_DAYLIGHT_OFFSET_SEC;

float temperature = 0.0;
float humidity = 0.0;
String currentTime = "";

const char* ntpServer = "pool.ntp.org";
// const long gmtOffset_sec = 3 * 3600;        // your UTC offset in seconds, e.g. 7200 for UTC+2
// const int daylightOffset_sec = 0;    // 3600 if your region observes DST, else 0


void getWeather();
void renderFrame();
void setupTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void setup() {
  Serial.begin(9600);

  Serial.println("Connecting to WiFi..");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }

  setupTime();

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000);

  
  display.clearDisplay();

  renderFrame();
}
void loop() {
  bool updated = true;

  if (WiFi.status() == WL_CONNECTED) {
      getWeather();
      delay(1000); // Delay between requests
      updated = true;
    } else {
      Serial.println("WiFi not connected");
  }

   struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char buffer[12];
    strftime(buffer, sizeof(buffer), "%I:%M", &timeinfo);
    currentTime = String(buffer);
    updated = true;
  }

  if (updated){
    renderFrame();
    updated = false;
  }

  delay(500);
}

void getWeather(){
  HTTPClient http;

  // Open-Meteo: free, no API key required
  String url = "https://api.open-meteo.com/v1/forecast?latitude=" + String(latitude, 4) +
               "&longitude=" + String(longitude, 4) +
               "&current=temperature_2m,relative_humidity_2m,weather_code&timezone=auto";
               
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();

    JsonDocument doc; // ArduinoJson v7 syntax
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      // serializeJson(doc, Serial);
      
      temperature = doc["current"]["temperature_2m"];
      humidity = doc["current"]["relative_humidity_2m"];
    } else {
      Serial.print("JSON parse failed: ");
      Serial.println(error.c_str());
    }
  } else {
    Serial.print("HTTP GET failed, error code: ");
    Serial.println(httpCode);
  }

  http.end();
}

void renderFrame() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(20, 4);
  display.println("Smart Desk Info");
  
  
  display.setCursor(6, 20); //64 16 + 16 + 4padding
  display.print("Temprature: ");
  display.print(temperature);
  display.println(" C");
  
  
  display.setCursor(6, 36);
  display.print("Humidity: ");
  display.print(humidity);
  display.println(" %");
  
  
  display.setCursor(6, 50);
  display.print("Current Time: ");
  display.print(currentTime);
  display.println("");

  display.display();
}

