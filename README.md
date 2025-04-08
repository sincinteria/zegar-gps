GPS Clock with ESP32-S2 Mini & AT6558R (GP-02-Kit)
Advanced GNSS Time Sync with LCD Display

Project Banner

🔍 Project Overview
A high-precision GPS/GNSS clock using ESP32-S2 Mini and GP-02-Kit (AT6558R) with ceramic antenna. Features automatic timezone adjustment (CET/CEST), LCD backlight dimming, and multi-constellation support (GPS, GLONASS, Galileo, BeiDou).

✨ Key Features
✅ Multi-GNSS Support – AT6558R chip supports GPS, GLONASS, Galileo, BeiDou
✅ High-Accuracy Time Sync – Ceramic antenna ensures stable signal
✅ Automatic DST Adjustment – Switches between CET (UTC+1) & CEST (UTC+2)
✅ Adaptive LCD Backlight – Dims at night (21:00–06:00)
✅ Satellite Tracking – Shows locked satellites count
✅ PlatformIO + VSCode – Ready for professional development

🛠 Hardware Setup
📋 Required Components
Component	Model	Notes
MCU	ESP32-S2 Mini	Cheap & powerful Wi-Fi MCU
GNSS Module	GP-02-Kit (AT6558R)	Multi-constellation support
LCD	16x2 I2C (PCF8574)	Default address 0x27
Antenna	Built-in ceramic (GP-02-Kit)	No external antenna needed
🔌 Wiring Guide
ESP32-S2 Mini	GP-02-Kit	I2C LCD
GPIO8 (SDA)	–	SDA
GPIO9 (SCL)	–	SCL
GPIO17 (TX)	RX	–
GPIO18 (RX)	TX	–
3.3V	VCC	VCC
GND	GND	GND
📌 Note: The GP-02-Kit runs at 3.3V (do NOT use 5V!).

⚙️ Firmware Configuration
📂 PlatformIO Setup
Install Dependencies (platformio.ini):

ini
Copy
[env:esp32-s2-mini]
platform = espressif32
board = esp32-s2-mini
framework = arduino
monitor_speed = 115200
lib_deps =
    wire
    liquidcrystal/I2C LCD@^1.1.4
    tinygpsplus/TinyGPSPlus@^1.0.3
Modify GPS UART Settings (for AT6558R):

cpp
Copy
HardwareSerial gpsSerial(1);  // Use UART1 (RX=GPIO18, TX=GPIO17)
void setup() {
    gpsSerial.begin(9600, SERIAL_8N1, 18, 17);  // AT6558R default baudrate
}
Enable Multi-GNSS Mode (Optional)

cpp
Copy
// Send UBX command to enable GPS + GLONASS + BeiDou
const uint8_t ENABLE_GNSS[] = {0xB5, 0x62, 0x06, 0x3E, 0x0C, 0x00, 0x00, 0x00, 0x20, 0x07, 0x00, 0x08, 0x10, 0x00, 0x01, 0x00, 0x01, 0x01};
gpsSerial.write(ENABLE_GNSS, sizeof(ENABLE_GNSS));
(Uncomment if you need GLONASS/Galileo support.)

🚀 How It Works
GPS Initialization

The ESP32 reads NMEA data from the AT6558R module.

TinyGPS++ decodes UTC time and satellite count.

Time Synchronization

Syncs every 2 minutes (adjustable in code).

Auto-adjusts for summer/winter time.

LCD Display

Line 1: HH:MM:SS SAT:XX (satellite count)

Line 2: Day, DD.MM.YYYY (e.g., Mon, 01.01.2024)

Backlight Control

Bright mode (06:00–21:00): 250/255

Dim mode (21:00–06:00): 10/255

🔧 Troubleshooting
Issue	Solution
No GPS Signal	Ensure ceramic antenna has a clear sky view.
LCD Not Working	Check I2C address (0x27 or 0x3F).
Wrong Timezone	Modify TIME_ZONE_OFFSET_WINTER/SUMMER.
AT6558R Not Responding	Verify baudrate (9600 by default).
📜 License
MIT License – Free for personal/commercial use.

📌 Final Notes
For better accuracy, place the antenna near a window.

To reduce power, disable Wi-Fi (WiFi.mode(WIFI_OFF)).

Want Wi-Fi/NTP fallback? Let me know!

🚀 Happy Hacking!
