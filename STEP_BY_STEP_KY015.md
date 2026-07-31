# Air Quality Coach: Step-by-step (DHT11 G / V / D)

You have: **ESP32** + **DHT11 module (G V D)** + **3 LEDs** + **resistors** + **Arduino IDE**

Wiring diagram image: `dht11-gvd-wiring-diagram.png` (same folder)

---

## Part A: Know your DHT11 pins

Your board has a blue DHT11 sensor and **3 pins labeled**:

| Pin label | Meaning | Connect to ESP32 |
|-----------|---------|------------------|
| **G** | Ground | **GND** |
| **V** | Power (VCC) | **5V** |
| **D** | Data | **GPIO4** |

```text
        DHT11 module (front view)
   ┌─────────────────────┐
   │      [ DHT11 ]      │
   │                     │
   │   G     V     D     │
   └───┬─────┬─────┬─────┘
       │     │     │
      GND   5V   DATA
```

Do not use old docs that say **S / + / -**. Your module is **G / V / D**.

---

## Part B: Wireframe (full circuit)

```text
                         USB to laptop
                              |
                              v
                    ┌──────────────────┐
                    │      ESP32       │
                    │                  │
                    │  5V  ────────────┼──────────► DHT11 (V)
                    │  GND ────────────┼──┬───────► DHT11 (G)
                    │  GPIO4 ──────────┼──│───────► DHT11 (D)
                    │                  │  │
                    │  GPIO25 ──[220Ω]─┼──│──► GREEN LED  ──┐
                    │  GPIO26 ──[220Ω]─┼──│──► YELLOW LED ─┤
                    │  GPIO27 ──[220Ω]─┼──│──► RED LED ────┤
                    │                  │  │                │
                    └──────────────────┘  └────────────────┘
                                              common GND
```

### LED detail

```text
GPIO25 ─────[ 220Ω resistor ]─────►|──── GND
                              LED long leg   short leg
                              (anode +)      (cathode -)
```

| Color  | GPIO  | Resistor | LED to |
|--------|-------|----------|--------|
| Green  | 25    | 220Ω     | GND    |
| Yellow | 26    | 220Ω     | GND    |
| Red    | 27    | 220Ω     | GND    |

---

## Part C: Build order

### Step 1: Unplug power
Remove USB from the ESP32 before wiring.

### Step 2: Put ESP32 on the breadboard
Seat the ESP32 so both pin rows have free holes (or use jumpers).

### Step 3: Wire DHT11 (G V D)
1. **G** → ESP32 **GND** (black)
2. **V** → ESP32 **5V** (red)
3. **D** → ESP32 **GPIO4** (yellow)

### Step 4 to 6: Wire LEDs
- Green: GPIO25 → 220Ω → LED long leg → short leg → GND
- Yellow: GPIO26 → same pattern
- Red: GPIO27 → same pattern

### Step 7: Double-check
- [ ] **G** is on **GND**
- [ ] **V** is on **5V**
- [ ] **D** is on **GPIO4**
- [ ] All LED short legs on **GND**
- [ ] No pins shorted together

---

## Part D: Arduino IDE

### Step 8: Board + port
1. Plug USB
2. Board: **ESP32 Dev Module**
3. Port: your ESP32 port (often `/dev/ttyUSB0`)

### Step 9: Install DHT library
Install **DHT sensor library** (Adafruit) and **Adafruit Unified Sensor**.

### Step 10: Open sketch
Open `air_quality_coach.ino` and confirm:

```cpp
#define DHTPIN   4
#define DHTTYPE  DHT11
```

### Step 11: Upload
Click Upload. Hold **BOOT** if it sticks on Connecting.

### Step 12: Serial Monitor
Baud **115200**. You should see:

```text
SoftAP started
SSID: AirCoach
IP:   192.168.4.1
Wiring: G->GND, V->5V, D->GPIO4
T=23.0C  H=48.0%  Air OK
```

---

## Part E: Phone test

| Item | Value |
|------|-------|
| Wi-Fi name | **AirCoach** |
| Password | **openwindow** |
| Page URL | **http://192.168.4.1/** |

Join AirCoach, open the URL, breathe on the DHT11 to raise humidity.

---

## Troubleshooting

| Symptom | Fix |
|---------|-----|
| Upload won’t connect | Hold **BOOT**, try another USB data cable |
| Always `Sensor error` | Recheck **G/V/D** (not S/+/-); try V on 5V; wait 2s after boot |
| All LEDs dark | Common GND missing; LED legs reversed |
| Phone can’t load page | Stay on AirCoach; open exactly `http://192.168.4.1` |
