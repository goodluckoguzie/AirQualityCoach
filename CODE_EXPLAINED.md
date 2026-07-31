# Full Code Explanation: `air_quality_coach.ino`

This document explains every part of the Air Quality Coach firmware in plain language.

**What the program does end-to-end**

1. Starts the ESP32
2. Turns on a Wi-Fi hotspot named `AirCoach`
3. Reads temperature and humidity from the DHT11
4. Decides if air is OK, stuffy, or needs a window open
5. Lights green / yellow / red LEDs
6. Serves a phone web page at `http://192.168.4.1`
7. Repeats sensor reads every 2.5 seconds

### Phone login credentials

| Item | Value |
|------|-------|
| Wi-Fi name (SSID / login) | `AirCoach` |
| Password | `openwindow` |
| Page URL | `http://192.168.4.1/` |

Code location:

```cpp
const char* AP_SSID = "AirCoach";
const char* AP_PASS = "openwindow";
```

---

## Big picture (data flow)

```text
DHT11 (G/V/D)
    |
    | humidity + temperature
    v
ESP32 decides level (0 / 1 / 2)
    |
    +-----> LEDs (green / yellow / red)
    |
    +-----> SoftAP Wi-Fi "AirCoach"
    |
    +-----> Web page on phone (/ and /json)
    |
    +-----> Serial Monitor log
```

---

## 1. Header comment and includes

```cpp
#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
```

| Library | Why we need it |
|---------|----------------|
| `WiFi.h` | Create the ESP32 Wi-Fi hotspot (SoftAP) |
| `WebServer.h` | Run a tiny website on the ESP32 |
| `DHT.h` | Talk to the DHT11 temperature/humidity sensor |

Without the correct board (**ESP32 Dev Module**), `WebServer.h` will not be found.

---

## 2. Pin definitions

```cpp
#define DHTPIN   4
#define DHTTYPE  DHT11

#define LED_GREEN  25
#define LED_YELLOW 26
#define LED_RED    27
```

These tell the ESP32 which physical pins are used.

| Name | GPIO | Connected to |
|------|------|--------------|
| `DHTPIN` | 4 | Sensor **D** (data) |
| `LED_GREEN` | 25 | Green LED (+ resistor) |
| `LED_YELLOW` | 26 | Yellow LED (+ resistor) |
| `LED_RED` | 27 | Red LED (+ resistor) |

Your sensor labels:

| Sensor | ESP32 |
|--------|-------|
| **G** | GND |
| **V** | 5V (or 3V3) |
| **D** | GPIO4 |

`DHTTYPE DHT11` must match your blue DHT11 sensor. Do not set `DHT22` unless you have that model.

---

## 3. Humidity thresholds

```cpp
const float HUM_OK      = 55.0;
const float HUM_STUFFY  = 70.0;
```

| Humidity | Level | Meaning | LED |
|----------|-------|---------|-----|
| below 55% | 0 | Air OK | Green |
| 55% to 69.9% | 1 | Getting stuffy | Yellow |
| 70% or more | 2 | OPEN THE WINDOW | Red |

You can change these numbers for your room or demo.

---

## 4. SoftAP Wi-Fi settings

```cpp
const char* AP_SSID = "AirCoach";
const char* AP_PASS = "openwindow";
```

The ESP32 creates its own Wi-Fi network. Phones join that network. No home router is required.

| Setting | Value |
|---------|-------|
| Network name | `AirCoach` |
| Password | `openwindow` (must be at least 8 characters) |
| Phone URL | `http://192.168.4.1` |

`192.168.4.1` is the default SoftAP IP for ESP32.

---

## 5. Global objects and state

```cpp
DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);
```

- `dht` = sensor object on GPIO4, DHT11 protocol
- `server(80)` = web server on HTTP port 80 (normal browser port)

```cpp
float temperatureC = NAN;
float humidity = NAN;
String statusText = "Starting...";
String advice = "Warming up sensor";
int level = 0;
```

These store the latest reading and message shown on LEDs / phone / Serial.

- `NAN` means "not a number yet" (or sensor failed)
- `level` is 0, 1, or 2

```cpp
unsigned long lastReadMs = 0;
const unsigned long READ_EVERY_MS = 2500;
```

DHT11 is slow. We only read it about every 2.5 seconds. Reading faster often causes errors.

---

## 6. `setLights(int lvl)`

```cpp
void setLights(int lvl) {
  digitalWrite(LED_GREEN,  lvl == 0 ? HIGH : LOW);
  digitalWrite(LED_YELLOW, lvl == 1 ? HIGH : LOW);
  digitalWrite(LED_RED,    lvl == 2 ? HIGH : LOW);
}
```

Turns on exactly one LED:

- level 0 → green only
- level 1 → yellow only
- level 2 → red only

`HIGH` = LED on, `LOW` = LED off (with common cathode wiring through resistor to GPIO).

---

## 7. `updateFromSensor()`

This is the decision brain.

### Step A: read sensor

```cpp
float h = dht.readHumidity();
float t = dht.readTemperature();
```

Asks the DHT11 for humidity (`h`) and temperature in Celsius (`t`).

### Step B: handle failure

```cpp
if (isnan(h) || isnan(t)) {
  statusText = "Sensor error";
  ...
}
```

If the sensor does not reply (bad wiring, power, timing), values are `nan`. We show yellow and ask you to check wiring.

### Step C: save good values and classify

```cpp
if (h < HUM_OK) { ... Air OK ... }
else if (h < HUM_STUFFY) { ... Getting stuffy ... }
else { ... OPEN THE WINDOW ... }
```

Then `setLights(level)` updates the hardware.

---

## 8. `colorForLevel()`

```cpp
String colorForLevel() {
  if (level == 0) return "#22c55e"; // green
  if (level == 1) return "#eab308"; // yellow
  return "#ef4444";                // red
}
```

Returns a CSS color for the phone page badge so the web UI matches the LED color.

---

## 9. `handleRoot()` (phone home page)

When someone opens `http://192.168.4.1`, this function builds an HTML page and sends it.

Important pieces:

| Piece | Meaning |
|-------|---------|
| `meta refresh content="3"` | Browser reloads every 3 seconds |
| `R"HTML( ... )HTML"` | Raw string so HTML is easy to write in C++ |
| `html += statusText` | Insert live status into the page |
| `server.send(200, "text/html", html)` | HTTP 200 OK + HTML body |

So the phone page is not a separate website file. The ESP32 builds it in memory each time.

---

## 10. `handleJson()` (machine-readable data)

```cpp
server.send(200, "application/json", json);
```

Opens at `http://192.168.4.1/json`.

Example:

```json
{
  "humidity": 48.0,
  "temperature": 23.0,
  "level": 0,
  "status": "Air OK",
  "advice": "No action needed"
}
```

Useful later for apps, dashboards, or demos.

---

## 11. `setup()` (runs once at boot)

Order of startup:

1. **`Serial.begin(115200)`**  
   Start USB serial logging. Serial Monitor must also be 115200.

2. **Configure LED pins as outputs**  
   Then set yellow while starting.

3. **`pinMode(DHTPIN, INPUT_PULLUP)` + `dht.begin()`**  
   Prepare the data line and start the DHT library.

4. **`WiFi.mode(WIFI_AP)` + `WiFi.softAP(...)`**  
   Create the `AirCoach` hotspot.

5. **Print IP / SSID to Serial**  
   Confirms SoftAP started.

6. **`server.on("/", handleRoot)`**  
   Map URL `/` to the HTML page function.

7. **`server.on("/json", handleJson)`**  
   Map URL `/json` to JSON.

8. **`server.begin()`**  
   Start listening for phone requests.

9. **`delay(2500)`**  
   Give DHT11 time to wake up.

10. **`updateFromSensor()`**  
    Take the first reading.

---

## 12. `loop()` (runs forever)

```cpp
void loop() {
  server.handleClient();

  if (millis() - lastReadMs >= READ_EVERY_MS) {
    lastReadMs = millis();
    updateFromSensor();
    Serial.printf("T=%.1fC  H=%.1f%%  %s\n", ...);
  }
}
```

### `server.handleClient()`
Must run often. If a phone asks for the page, this answers it.

### Timed sensor update
`millis()` is milliseconds since boot. Every 2500 ms we:

1. Read the sensor again
2. Update LEDs / status
3. Print one line to Serial, for example:
   ```text
   T=23.0C  H=48.0%  Air OK
   ```

We do **not** use `delay(2500)` in `loop()` because that would freeze the web server and make the phone page slow or broken.

---

## How to demo it (talking points)

1. "This ESP32 is an IoT node: sense, decide, act, and connect."
2. "The DHT11 senses humidity, an invisible comfort/air problem."
3. "Traffic-light LEDs make the advice visible across the room."
4. "SoftAP means visitors use their phones with no venue Wi-Fi setup."
5. "Breathe on the sensor live: humidity rises, yellow then red, phone updates."

---

## Common edits you may want

| Goal | What to change |
|------|----------------|
| More sensitive demo | Lower `HUM_OK` / `HUM_STUFFY` (example: 45 and 55) |
| Different Wi-Fi name | Change `AP_SSID` / `AP_PASS` |
| Sensor on another pin | Change `DHTPIN` and move the **D** wire |
| Add a buzzer on red | `pinMode` a buzzer pin and `digitalWrite` when `level == 2` |

---

## File map

| File | Role |
|------|------|
| `air_quality_coach.ino` | Firmware explained above |
| `CODE_EXPLAINED.md` | This document |
| `STEP_BY_STEP_KY015.md` | Build / wiring walkthrough |
| `README.md` | Quick start |
| `dht11-gvd-wiring-diagram.png` | Wiring image (G / V / D) |
| `sketch.yaml` | Forces ESP32 board profile in Arduino IDE 2 |
