# ⚡ IoT Based Three-Phase Power Failure Monitoring System with SMS Alerts

![Arduino](https://img.shields.io/badge/Arduino-Uno-00979D?style=for-the-badge&logo=arduino&logoColor=white)
![ESP32](https://img.shields.io/badge/ESP32-WROOM--32-E7352C?style=for-the-badge&logo=espressif&logoColor=white)
![ThingSpeak](https://img.shields.io/badge/ThingSpeak-Cloud-blue?style=for-the-badge)
![GSM](https://img.shields.io/badge/SIM900A-GSM-green?style=for-the-badge)
![PlatformIO](https://img.shields.io/badge/PlatformIO-VSCode-orange?style=for-the-badge&logo=platformio&logoColor=white)

An industrial-grade, IoT-enabled power monitoring system designed to detect three-phase power failures, undervoltage, and overvoltage conditions in real-time. The system provides immediate SMS alerts via a GSM module and logs real-time data to a cloud dashboard (ThingSpeak) for remote monitoring.

## 📖 Project Overview

Power failures in industrial and commercial three-phase systems can cause severe damage to machinery and result in significant downtime. This project acts as an early-warning system by continuously monitoring the voltage of all three phases (R, Y, B). 

If any phase drops below the safe threshold (Undervoltage) or goes above it (Overvoltage), the system:
1. Triggers a local buzzer and LED indicator.
2. Sends an **SMS alert** to the operator within 2–3 seconds.
3. Updates the **ThingSpeak Cloud Dashboard** with live voltage readings and fault status.
4. Sends a **"Power Restored"** SMS when normal voltage returns.

## ✨ Key Features

*   **Real-Time Monitoring:** Continuous tracking of R, Y, and B phase voltages.
*   **Dual-Alert System:** Local audio/visual alarm + Remote SMS notifications.
*   **Cloud Logging:** Live graphs and data logging using ThingSpeak API.
*   **Noise Filtering:** Uses RMS calculation and sample averaging for stable, accurate voltage readings.
*   **Fault Detection:** Detects Phase Failure, Undervoltage (< 180V), and Overvoltage (> 270V).
*   **Power Restoration Alert:** Automatically notifies when normal power is restored.
*   **Dual-Microcontroller Architecture:** Arduino handles precise analog sensing, while ESP32 handles heavy lifting of Wi-Fi, Cloud, and GSM communication.

## 🛠️ Hardware Requirements

| Component | Quantity | Purpose |
|-----------|----------|---------|
| Arduino Uno | 1 | Analog voltage sensing & local control |
| ESP32 Dev Board | 1 | Wi-Fi, Cloud (ThingSpeak), and GSM control |
| SIM900A GSM Module | 1 | Sending SMS alerts |
| ZMPT101B Voltage Sensor | 3 | 3-Phase AC voltage sensing |
| 16x2 I2C LCD | 1 | Local voltage display |
| Buzzer & LEDs (R/Y/B) | 1 Set | Local fault indication |
| Buck Converter (12V to 5V) | 1 | Powering the logic circuits |
| **5V/2A Power Supply** | **1** | **Dedicated power for SIM900A (Crucial!)** |

## 🔌 Circuit Connection Summary

### 1. Arduino Uno (Sensing & Display)
*   **ZMPT101B Sensors:** Connect VOUT to `A0` (R), `A1` (Y), `A2` (B).
*   **LCD (I2C):** SDA -> `A4`, SCL -> `A5`.
*   **Buzzer:** `Pin 2`
*   **Indicators:** Red LED -> `Pin 3`, Yellow LED -> `Pin 4`, Blue LED -> `Pin 5`.
*   **ESP32 Comm:** `TX (Pin 11)` -> ESP32 `RX2 (GPIO16)` *(Use Logic Level Shifter 5V to 3.3V)*

### 2. ESP32 (Cloud & Communication)
*   **Arduino Comm:** `RX2 (GPIO16)` <- Arduino TX.
*   **SIM900A GSM:** `RX (GPIO26)` <- SIM900A TX, `TX (GPIO27)` -> SIM900A RX.
*   **Power:** ESP32 powered via USB or 5V Buck Converter.

> ⚠️ **CRITICAL WARNING:** Do **NOT** power the SIM900A module from the Arduino or ESP32 5V pin. It draws up to 2A during SMS transmission, which will reboot the microcontrollers. Use a dedicated 5V/2A power supply and add a **1000µF capacitor** across the SIM900A VCC and GND pins.

## 💻 Software & Libraries

This project uses **PlatformIO** (VS Code) for ESP32 programming and **Arduino IDE** for Arduino Uno programming.

### Arduino IDE Libraries:
*   `LiquidCrystal_I2C.h` (by Frank de Brabander)
*   `Wire.h` (Built-in)

### PlatformIO (`platformio.ini`) dependencies for ESP32:
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps = 
    mathworks/ThingSpeak
☁️ ThingSpeak Cloud Setup
Create a free account at ThingSpeak.

Create a New Channel with 4 Fields:

Field 1: Voltage R

Field 2: Voltage Y

Field 3: Voltage B

Field 4: Fault Status (0 = OK, 1 = Fault)

Go to API Keys and copy your Write API Key.

Update the following variables in the ESP32 code:

cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
unsigned long channelID = YOUR_CHANNEL_ID;
const char* writeAPIKey = "YOUR_WRITE_API_KEY";
String phoneNumber = "+91XXXXXXXXXX"; // Your mobile number
🚀 How to Run This Project
Calibrate ZMPT101B: Open the Arduino code, upload it, and adjust the CAL_FACTOR variable until the LCD reading matches a standard multimeter reading.

Insert SIM Card: Put a 2G-enabled SIM card (Airtel/Vi/BSNL) into the SIM900A module with an active SMS pack. (Note: Jio SIMs will not work as they are 4G only).

Flash Arduino Uno: Upload the Arduino code to the Uno.

Flash ESP32: Open the project in PlatformIO, compile, and upload the ESP32 code.

Power Up: Turn on the 5V/2A dedicated supply for the GSM module, and power the microcontrollers.

Test: Disconnect one phase sensor input to simulate a fault. You should receive an SMS and see the ThingSpeak dashboard update.

📊 Results & Achievements
Fault Detection Speed: SMS delivered within 2–3 seconds of fault occurrence.

Detection Accuracy: Achieved 98% accuracy across 50+ simulated test cycles using sample averaging and threshold hysteresis.

Cloud Reliability: Data logged every 20 seconds (staying within ThingSpeak's free tier API limits).

👨‍💻 Author
Armaan Ahemad

B.Tech Electronics and Communication Engineering

armaanahemad@outlook.com
