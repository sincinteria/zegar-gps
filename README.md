# 🕒 GPS Clock with ESP32-S2 Mini & AT6558R GNSS

![Project Banner](https://via.placeholder.com/800x300?text=ESP32-S2+Mini+GPS+Clock)  
*Precise multi-GNSS time synchronization with automatic DST adjustment*

## 📌 Features
- ✅ **Multi-constellation support** (GPS/GLONASS/Galileo/BeiDou via AT6558R)
- ✅ **Ceramic antenna** for stable signal reception
- ✅ **Automatic timezone switching** (CET/CEST)
- ✅ **Adaptive LCD backlight** (dimming at night)
- ✅ **PlatformIO-ready** for VSCode development
- ✅ **Low-power design** (ESP32-S2 Mini optimized)

## 🛠 Hardware Requirements
| Component | Specification |
|-----------|---------------|
| MCU | ESP32-S2 Mini |
| GNSS Module | GP-02-Kit (AT6558R) |
| Display | 16x2 I2C LCD (PCF8574 @ 0x27) |
| Antenna | Integrated ceramic patch |

## 🔌 Wiring Diagram
```plaintext
ESP32-S2 Mini  ↔  GP-02-Kit  ↔  I2C LCD
───────────────────────────────────────
GPIO18 (RX)    →   TX
GPIO17 (TX)    →   RX
GPIO8 (SDA)    →   SDA
GPIO9 (SCL)    →   SCL
3.3V           →   VCC
GND            →   GND

🚀 Quick Start
Clone this repository

Open in VSCode with PlatformIO

Connect hardware as per wiring diagram

Upload and monitor serial port (115200 baud)

🌟 Advanced Features
Configurable sync interval (default: 2 minutes)

NTP fallback (Wi-Fi capable)

Battery backup support (optional)

📜 License
MIT License - Free for personal and commercial use
