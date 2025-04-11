# 🕒 GPS Clock with ESP32-S2 Mini & AT6558R GNSS & LCD Display

![Project Banner](https://via.placeholder.com/800x300?text=ESP32-S2+Mini+GPS+Clock)  
*Precise multi-GNSS time synchronization with automatic DST adjustment*

## 👀 Overview
This project implements a digital clock on an ESP32 microcontroller that synchronizes time via GPS signals. It displays current time, date, day of the week, and satellite status on a 16x2 I2C LCD display. The display's backlight automatically dims during nighttime hours for better visibility in low-light conditions.

## 📌 Features
- ✅ **Multi-constellation support** (GPS/GLONASS/Galileo/BeiDou via AT6558R)
- ✅ **Ceramic antenna** for stable signal reception
- ✅ **Automatic time synchronization using GPS data**
- ✅ **Automatic backlight brightness adjustment** based on time of day
- ✅ **Adaptive LCD backlight** (dimming at night)
- ✅ **PlatformIO-ready** for VSCode development
- ✅ **Low-power design** (ESP32-S2 Mini optimized)
- ✅ **Periodic GPS re-synchronization**
- ✅ **Daily automatic restart** at 5:00 AM for stability
- ✅ **LCD display showing:**
  - Current time in HH:MM:SS format
  - Current date in DD.MM.YYYY format
  - Day of the week in Polish (abbreviated)
  - Number of connected GPS satellites
  - Support for Polish special characters on the LCD

## 🛠 Hardware Requirements
| Component | Specification |
|-----------|---------------|
| MCU | ESP32-S2 Mini |
| GNSS Module | GP-02-Kit (AT6558R) |
| Display | 16x2 I2C LCD (PCF8574 @ 0x27) |
| Antenna | Integrated ceramic patch |

# 🔌 Wiring Diagram - ESP32-S2 Mini + GP-02-Kit + LCD
```plaintext
───────────────────────────────────────────
| ESP32-S2 Mini | GP-02-Kit | I2C LCD 16x2 |
|---------------|-----------|--------------|
| 5V (Pin VBUS) |   VCC     |    VCC       |
| GND (Pin 3)   |   GND     |    VSS       |
| GPIO17 (TX)   |   RX      |     -        |
| GPIO18 (RX)   |   TX      |     -        |
| GPIO8 (SDA)   |    -      |    SDA       |
| GPIO9 (SCL)   |    -      |    SCL       |
| GPIO10        |    -      | ANODE LCD*   |
───────────────────────────────────────────
* Remove the jumper marked with a red dot.
Then, connect GPIO 10 of the ESP32 to the LCD backlight control pin to enable PWM dimming.
```
![image](https://github.com/user-attachments/assets/3fb7b169-b397-40a9-9b90-3bbae7ca0044)


## ⚙️ Operation
1. On startup, the device will display "RTC GPS Sync" and then wait for a valid GPS signal
2. The screen will show "Czekam na GPS..." with satellite count and fix status while searching
3. After establishing a GPS fix with at least 3 satellites, time will be synchronized
4. The main screen displays the current time, satellite count, date, and day of the week
5. Time is automatically re-synchronized with GPS every hour
6. The display backlight dims between 21:00 and 6:00
7. The device automatically restarts every day at 5:00 AM

## 🎨 Customization
The following parameters can be adjusted in the code:

### LCD Configuration
```cpp
LiquidCrystal_I2C lcd(0x27, 16, 2); // Change I2C address if needed
```

### GPS Serial Configuration
```cpp
#define RX_PIN 18  // GPS TX connects to this ESP32 pin
#define TX_PIN 17  // GPS RX connects to this ESP32 pin
```

### Backlight Settings
```cpp
const int BACKLIGHT_PIN = 10;        // PWM pin for backlight control
const int BRIGHT_BACKLIGHT = 250;    // Daytime brightness (0-255)
const int DIM_BACKLIGHT = 10;        // Nighttime brightness (0-255)
const int NIGHT_HOUR_START = 21;     // Hour to start dimming (24h format)
const int NIGHT_HOUR_END = 6;        // Hour to stop dimming (24h format)
```

### GPS Synchronization
```cpp
const uint32_t SYNC_INTERVAL = 3600000UL;  // Re-sync interval in milliseconds (1 hour)
const int MIN_SATELLITES = 3;  // Minimum satellites required for a valid fix
```

## 🐛 Troubleshooting
- If the LCD shows "GPS Sync FAIL!", check your GPS module's connections and ensure it has a clear view of the sky
- If special characters aren't displaying correctly, verify the I2C connection and address
- If time appears incorrect, verify the time zone adjustment in the code (currently +2 hours)

## 📦 Dependencies
- Arduino.h
- wire.h
- LiquidCrystal_I2C
- TinyGPS++
- time.h

## 🚀 Quick Start
Clone this repository
Open in VSCode with PlatformIO
Connect hardware as per wiring diagram
Upload and monitor serial port (115200 baud)

## 🌟 Advanced Features
Configurable sync interval (default: 1 hour)
Battery backup support (optional)

## 📜 License
MIT License - Free for personal and commercial use
