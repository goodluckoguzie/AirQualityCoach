# Air Quality Coach: Step-by-step (KY-015 DHT11)

You have: **ESP32** + **KY-015 (DHT11)** + **3 LEDs** + **resistors** + **Arduino IDE**

Wiring diagram image: `ky015-wiring-diagram.png` (same folder)

---

## Part A: Know your KY-015 pins

The **KY-015** is a small board with a blue DHT11 can and **3 pins**.

With the **sensor facing you** and pins at the bottom, left → right is usually:

| Pin label | Meaning | Connect to ESP32 |
|-----------|---------|------------------|
| **S** | Signal / DATA | **GPIO4** |
| **+** (middle) | Power (VCC) | **3V3** |
| **-** | Ground | **GND** |

```
        KY-015 (front view)
   ┌─────────────────────┐
   │      [ DHT11 ]      │
   │                     │
   │   S     +     -     │
   └───┬─────┬─────┬─────┘
       │     │     │
     DATA   VCC   GND
```

If your board prints `S` / middle / `-`, use that. Do **not** guess VCC/GND. Wrong power can damage the module.

---

## Part B: Wireframe (full circuit)

```
                         USB to laptop
                              │
                              ▼
                    ┌──────────────────┐
                    │      ESP32       │
                    │                  │
                    │  3V3 ────────────┼──────────► KY-015 (+)
                    │  GND ────────────┼──┬───────► KY-015 (-)
                    │  GPIO4 ──────────┼──│───────► KY-015 (S)
                    │                  │  │
                    │  GPIO25 ──[220Ω]─┼──│──► GREEN LED  ──┐
                    │  GPIO26 ──[220Ω]─┼──│──► YELLOW LED ─┤
                    │  GPIO27 ──[220Ω]─┼──│──► RED LED ────┤
                    │                  │  │                │
                    └──────────────────┘  └────────────────┘
                                              common GND
```

### LED detail (one LED shown; repeat for all three)

```
GPIO25 ─────[ 220Ω resistor ]─────►|──── GND
                              LED long leg   short leg
                              (anode +)      (cathode -)
```

| Color  | GPIO  | Resistor | LED to |
|--------|-------|----------|--------|
| Green  | 25    | 220Ω     | GND    |
| Yellow | 26    | 220Ω     | GND    |
| Red    | 27    | 220Ω     | GND    |

Use **220Ω** (red-red-brown) or **330Ω**. Do not skip the resistor.

---

## Part C: Build order (do this in order)

### Step 1: Unplug power
Remove USB from the ESP32 before wiring. Safer.

### Step 2: Put ESP32 on the breadboard
Seat the ESP32 so both rows of pins have free holes beside them (or use male-female jumpers to a separate breadboard).

### Step 3: Wire KY-015
1. KY-015 **+** → ESP32 **3V3** (red jumper)
2. KY-015 **-** → ESP32 **GND** (black jumper)
3. KY-015 **S** → ESP32 **GPIO4** / pin labeled **D4** on many Freenove boards (yellow jumper)

### Step 4: Wire green LED
1. Resistor from **GPIO25** to breadboard row
2. From that row to **long leg** of green LED
3. **Short leg** of green LED to **GND** rail

### Step 5: Wire yellow LED
Same pattern: **GPIO26** → resistor → yellow LED long leg → short leg → **GND**

### Step 6: Wire red LED
Same pattern: **GPIO27** → resistor → red LED long leg → short leg → **GND**

### Step 7: Double-check
- [ ] KY-015 + is on **3V3**, not 5V (safer for first test)
- [ ] KY-015 S is on **GPIO4**
- [ ] All LED short legs on **GND**
- [ ] No two ESP32 pins shorted together
- [ ] USB cable is a **data** cable (not charge-only)

---

## Part D: Arduino IDE (software)

### Step 8: Board + port
1. Plug USB into ESP32 and laptop
2. Arduino IDE → **Tools → Board → ESP32 Arduino → ESP32 Dev Module**
3. **Tools → Port** → pick the new serial port  
   - Linux often `/dev/ttyUSB0` or `/dev/ttyACM0`

### Step 9: Install DHT library
1. **Sketch → Include Library → Manage Libraries**
2. Search **DHT sensor library**
3. Install **DHT sensor library** by **Adafruit**
4. If asked, also install **Adafruit Unified Sensor**

### Step 10: Open the sketch
Open:

`Tutorial/air-quality-coach/air_quality_coach.ino`

Confirm these lines (already set for your sensor):

```cpp
#define DHTPIN   4
#define DHTTYPE  DHT11
```

### Step 11: Upload
1. Click **Upload**
2. If it fails stuck on “Connecting…”, hold the ESP32 **BOOT** button until upload starts, then release
3. Wait for **Done uploading**

### Step 12: Serial Monitor
1. **Tools → Serial Monitor**
2. Set baud to **115200**
3. You should see something like:

```text
SoftAP started
SSID: AirCoach
IP:   192.168.4.1
T=23.0C  H=48.0%  Air OK
```

- Green LED should be on if humidity is below 55%
- If you see `Sensor error`, re-check KY-015 S/+/- wiring

---

## Part E: Phone test

### Phone login

| Item | Value |
|------|-------|
| Wi-Fi name (SSID / login) | **AirCoach** |
| Password | **openwindow** |
| Page URL | **http://192.168.4.1/** |

### Step 13: Connect phone Wi-Fi
- Network: **AirCoach**
- Password: **openwindow**

(Your phone may say "No internet". That is normal. Stay connected.)

### Step 14: Open the page
In the phone browser go to:

**http://192.168.4.1**

You should see humidity, temperature, and a status badge.

### Step 15: Make it go red (demo trick)
Hold the KY-015 gently and **breathe warm moist air** onto the blue DHT11 can for a few seconds.

Expected:
1. Humidity rises on the page
2. Yellow LED → then Red LED
3. Badge text: **OPEN THE WINDOW**

---

## What “done” looks like

| State | Humidity | LED | Phone text |
|-------|----------|-----|------------|
| OK | &lt; 55% | Green | Air OK |
| Stuffy | 55-70% | Yellow | Getting stuffy |
| Alert | ≥ 70% | Red | OPEN THE WINDOW |

---

## Troubleshooting (quick)

| Symptom | Fix |
|---------|-----|
| Upload won’t connect | Hold **BOOT**, try other USB cable/port |
| Always `Sensor error` | Swap S and check +/− ; wait 2s after boot; confirm DHT11 library |
| All LEDs dark | Common GND missing; LEDs reversed (swap legs) |
| Phone can’t load page | Still on AirCoach? Use exactly `http://192.168.4.1` |
| Humidity stuck | Don’t touch DATA wire while reading; avoid very long jumpers |

---

## After this works

Tell me:
1. What Serial Monitor prints
2. Which LED is on at rest
3. Whether the phone page loads

Then we can tune thresholds or add a **buzzer on red** for more wow.
