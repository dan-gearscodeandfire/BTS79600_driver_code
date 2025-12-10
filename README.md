# ESP32 MCPWM Motor Driver – IBT-2 (BTS7960) Control

This project demonstrates how to control a high-current DC motor using an **ESP32** and the **IBT-2 (BTS7960)** 43A H-bridge motor driver.  
It uses the ESP32’s dedicated **MCPWM peripheral** for jitter-free, hardware-accelerated PWM output.

The included Arduino sketch provides:

- Bidirectional motor control  
- Speed control (0–100%)  
- Smooth ramping demo  
- Fully documented motor control class (`MyMotor`)  

This example is ideal for robotics, linear actuators, high-current drive systems, or any DC motor requiring more power than L298N / DRV8833 drivers can handle.

---

## ⭐ Features

- ✔️ Uses ESP32 **MCPWM hardware** (not LEDC timers)
- ✔️ Controls a **43A dual half-bridge BTS7960 module**
- ✔️ Forward & reverse logic using independent PWM channels
- ✔️ Clean object-oriented design
- ✔️ Fully commented code for beginners and experts alike
- ✔️ Includes wiring tables for correct ESP32 → IBT-2 connections

---

# 🧠 How It Works

The IBT-2 (BTS7960) contains *two* half-bridges:

- **RPWM** → controls the "forward" MOSFETs  
- **LPWM** → controls the "reverse" MOSFETs  

The ESP32's MCPWM outputs two channels:

| MCPWM Channel | ESP32 GPIO | Function |
|---------------|------------|----------|
| MCPWM0A       | GPIO 4     | Forward PWM (RPWM) |
| MCPWM0B       | GPIO 15    | Reverse PWM (LPWM) |

The motor direction is determined by which side gets PWM:

- PWM on RPWM → Forward  
- PWM on LPWM → Reverse  
- Both 0% → Motor off (coast)

Both **R_EN** and **L_EN** must be held HIGH or the driver will not output any current.

---

# 🔌 Wiring Diagram (ESP32 → IBT-2)

### **IBT-2 Input Pins**
| Pin | Label | Description |
|-----|--------|-------------|
| 1 | RPWM  | Forward PWM (active high) |
| 2 | LPWM  | Reverse PWM (active high) |
| 3 | R_EN  | Enable forward driver |
| 4 | L_EN  | Enable reverse driver |
| 5 | R_IS  | Forward current sense |
| 6 | L_IS  | Reverse current sense |
| 7 | VCC   | +5 V logic supply |
| 8 | GND   | Signal ground |

### **ESP32 → IBT-2 Wiring**
| IBT-2 Pin | Connect To | Purpose |
|-----------|------------|---------|
| **RPWM (1)** | **GPIO 4** | Forward PWM (MCPWM0A) |
| **LPWM (2)** | **GPIO 15** | Reverse PWM (MCPWM0B) |
| **R_EN (3)** | 5V | Enable forward channel |
| **L_EN (4)** | 5V | Enable reverse channel |
| **R_IS (5)** | Optional ESP32 ADC | Current/fault |
| **L_IS (6)** | Optional ESP32 ADC | Current/fault |
| **VCC (7)**  | 5V | Logic power |
| **GND (8)**  | ESP32 GND | Shared ground |

### ASCII Diagram

```
        ESP32                                IBT-2 (BTS7960)
   +----------------+                       +----------------------+
   |                |                       |                      |
   |     GPIO 4 ----+---------------------->+ RPWM (Forward PWM)   |
   |                |                       |                      |
   |     GPIO 15 ---+---------------------->+ LPWM (Reverse PWM)   |
   |                |                       |                      |
   |       5V ------+---------------------->+ R_EN (Enable FWD)    |
   |       5V ------+---------------------->+ L_EN (Enable REV)    |
   |                |                       |                      |
   |      GND ------+---------------------->+ GND                  |
   +----------------+                       +----------------------+
```

---

# 📦 Included Class: `MyMotor`

### Constructor
```cpp
MyMotor motor(4, 15);
```

Maps:
- GPIO 4 → RPWM  
- GPIO 15 → LPWM  

### API

```cpp
void setSpeed(int speed);
```

| Input | Meaning |
|--------|---------|
| 100 | Full forward |
| 0 | Motor off |
| -100 | Full reverse |

---

# 🧪 Example Motion Pattern (in loop)

The included `loop()` performs:

1. Ramp forward 0 → 100%  
2. Hold  
3. Ramp down  
4. Ramp reverse 0 → -100%  
5. Hold  
6. Ramp down  

This ensures:
- Both directions tested  
- Smooth PWM control  
- Debug output prints to Serial Monitor  

---

# 🛠️ Requirements

- ESP32 Dev Module (any variant)
- Arduino IDE with ESP32 core
- IBT-2 / BTS7960 H-bridge (43A)
- External motor power supply (6–27 V)
- Shared GND between motor supply, IBT-2, and ESP32

---

# 📁 File Structure

```
/
├── Motor_IBT2_MCPWM.ino    # Main Arduino sketch with full documentation
├── README.md               # This documentation
```

---

# ⚠️ Safety Notes

- The IBT-2 can handle **43A**, but your wiring and power supply MUST also support this.  
- Always fuse your motor power line appropriately.  
- Never change wiring while powered.  
- Motors may kick unexpectedly during testing—secure your project.

---

# 🚀 Getting Started

1. Wire the ESP32 and IBT-2 according to the table above.  
2. Install ESP32 support in Arduino IDE.  
3. Upload the sketch.  
4. Open **Serial Monitor @ 115200**.  
5. Observe the directional test cycle.

You should see:

```
Forward Speed: 10%
Forward Speed: 20%
...
Reverse Speed: -100%
...
Motor Stopped
```

---

# 🧭 Future Improvements (Optional)

- Add closed-loop feedback using R_IS / L_IS pins  
- Add position sensing (rotary encoder)
- Add acceleration/deceleration profiles
- Add soft-start / braking logic
- Add Home Assistant MQTT control

---

# 📜 License

MIT License — free to use in commercial or personal projects.
