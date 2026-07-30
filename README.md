# Air Quality Coach — “Open the Window”

ESP32 + **KY-015 DHT11** + traffic-light LEDs + SoftAP phone page.

When humidity gets high, the red LED lights and your phone says **OPEN THE WINDOW**.

Built for **LaunchPoint 2026 IoT Club Showcase**.

---

## Wiring (KY-015)

![KY-015 DHT11 wiring diagram](ky015-wiring-diagram.png)

| KY-015 pin | ESP32 |
|------------|-------|
| **S** (signal) | GPIO4 |
| **+** (VCC) | 3V3 |
| **-** (GND) | GND |

| LED | ESP32 GPIO | Resistor |
|-----|------------|----------|
| Green | 25 | 220Ω |
| Yellow | 26 | 220Ω |
| Red | 27 | 220Ω |

LED: long leg → resistor side; short leg → GND.

---

## Quick start

1. Wire as above (unplug USB first)
2. Arduino IDE → Board **ESP32 Dev Module** → select Port
3. Install **DHT sensor library** (Adafruit) + **Adafruit Unified Sensor**
4. Open `air_quality_coach.ino` → Upload
5. Serial Monitor at **115200** → should show `SoftAP started`
6. Phone Wi‑Fi: **AirCoach** / **openwindow**
7. Browser: [http://192.168.4.1](http://192.168.4.1)
8. Breathe on the DHT11 → yellow/red + “OPEN THE WINDOW”

Full walkthrough: [`STEP_BY_STEP_KY015.md`](STEP_BY_STEP_KY015.md)

---

## How it works

1. DHT11 reads temperature + humidity every 2 seconds
2. Thresholds:
   - **&lt; 55%** → green / Air OK
   - **55–70%** → yellow / Getting stuffy
   - **≥ 70%** → red / OPEN THE WINDOW
3. ESP32 SoftAP serves a live HTML page (and `/json`) — no home router required

---

## Files

| File | Purpose |
|------|---------|
| `air_quality_coach.ino` | ESP32 firmware |
| `ky015-wiring-diagram.png` | Wiring diagram |
| `STEP_BY_STEP_KY015.md` | Detailed build guide |
| `README.md` | This file |

---

## Hardware

- ESP32 (Freenove / DevKit)
- KY-015 DHT11 module
- 3 LEDs + 220Ω resistors
- Breadboard + jumper wires

---

## License

MIT — free to use and adapt for demos and teaching.
