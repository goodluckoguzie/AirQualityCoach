/*
  Air Quality Coach — "Open the Window"
  ESP32 + DHT + traffic-light LEDs + SoftAP web page

  Phones connect to Wi-Fi: AirCoach
  Password:          openwindow
  Then open:         http://192.168.4.1
*/

#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

// ---------- PINS (change if your wiring differs) ----------
#define DHTPIN   4
#define DHTTYPE  DHT11   // KY-015 module = DHT11 (do not set DHT22)

#define LED_GREEN  25
#define LED_YELLOW 26
#define LED_RED    27

// ---------- THRESHOLDS (tune for your room) ----------
const float HUM_OK      = 55.0;  // below this = green
const float HUM_STUFFY  = 70.0;  // between OK and this = yellow; above = red

// ---------- SoftAP (no home Wi-Fi needed) ----------
const char* AP_SSID = "AirCoach";
const char* AP_PASS = "openwindow";  // min 8 chars

DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

float temperatureC = NAN;
float humidity = NAN;
String statusText = "Starting...";
String advice = "Warming up sensor";
int level = 0;  // 0=ok, 1=stuffy, 2=open window

unsigned long lastReadMs = 0;
const unsigned long READ_EVERY_MS = 2000;

void setLights(int lvl) {
  digitalWrite(LED_GREEN,  lvl == 0 ? HIGH : LOW);
  digitalWrite(LED_YELLOW, lvl == 1 ? HIGH : LOW);
  digitalWrite(LED_RED,    lvl == 2 ? HIGH : LOW);
}

void updateFromSensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    statusText = "Sensor error";
    advice = "Check DHT wiring / power";
    level = 1;
    setLights(level);
    return;
  }

  humidity = h;
  temperatureC = t;

  if (h < HUM_OK) {
    level = 0;
    statusText = "Air OK";
    advice = "No action needed";
  } else if (h < HUM_STUFFY) {
    level = 1;
    statusText = "Getting stuffy";
    advice = "Crack a window soon";
  } else {
    level = 2;
    statusText = "OPEN THE WINDOW";
    advice = "Ventilate now — humidity is high";
  }

  setLights(level);
}

String colorForLevel() {
  if (level == 0) return "#22c55e";
  if (level == 1) return "#eab308";
  return "#ef4444";
}

void handleRoot() {
  String html = R"HTML(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8"/>
  <meta name="viewport" content="width=device-width, initial-scale=1"/>
  <meta http-equiv="refresh" content="3"/>
  <title>AirCoach</title>
  <style>
    body { font-family: system-ui, sans-serif; background:#0b1220; color:#fff; margin:0; padding:24px; }
    .card { max-width:420px; margin:0 auto; background:#151d2e; border-radius:16px; padding:24px; }
    h1 { margin:0 0 8px; font-size:1.4rem; }
    .badge { display:inline-block; padding:8px 14px; border-radius:999px; font-weight:700; margin:12px 0; }
    .row { display:flex; justify-content:space-between; padding:10px 0; border-bottom:1px solid #243049; }
    .muted { color:#9aa6bf; font-size:0.9rem; }
  </style>
</head>
<body>
  <div class="card">
    <h1>Air Quality Coach</h1>
    <p class="muted">Live room reading · auto-refresh</p>
    <div class="badge" style="background:)HTML";

  html += colorForLevel();
  html += "\">";
  html += statusText;
  html += "</div>";

  html += "<div class=\"row\"><span>Humidity</span><strong>";
  html += isnan(humidity) ? "--" : String(humidity, 1) + " %";
  html += "</strong></div>";

  html += "<div class=\"row\"><span>Temperature</span><strong>";
  html += isnan(temperatureC) ? "--" : String(temperatureC, 1) + " &deg;C";
  html += "</strong></div>";

  html += "<p style=\"margin-top:18px\">";
  html += advice;
  html += "</p>";
  html += "<p class=\"muted\">Thresholds: OK &lt; ";
  html += String(HUM_OK, 0);
  html += "% · Open window &ge; ";
  html += String(HUM_STUFFY, 0);
  html += "%</p></div></body></html>";

  server.send(200, "text/html", html);
}

void handleJson() {
  String json = "{";
  json += "\"humidity\":" + (isnan(humidity) ? "null" : String(humidity, 1)) + ",";
  json += "\"temperature\":" + (isnan(temperatureC) ? "null" : String(temperatureC, 1)) + ",";
  json += "\"level\":" + String(level) + ",";
  json += "\"status\":\"" + statusText + "\",";
  json += "\"advice\":\"" + advice + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  setLights(1);

  dht.begin();

  WiFi.mode(WIFI_AP);
  bool ok = WiFi.softAP(AP_SSID, AP_PASS);
  Serial.println(ok ? "SoftAP started" : "SoftAP failed");
  Serial.print("SSID: ");
  Serial.println(AP_SSID);
  Serial.print("IP:   ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/json", handleJson);
  server.begin();

  delay(1500);  // DHT warm-up
  updateFromSensor();
}

void loop() {
  server.handleClient();

  if (millis() - lastReadMs >= READ_EVERY_MS) {
    lastReadMs = millis();
    updateFromSensor();
    Serial.printf("T=%.1fC  H=%.1f%%  %s\n", temperatureC, humidity, statusText.c_str());
  }
}
