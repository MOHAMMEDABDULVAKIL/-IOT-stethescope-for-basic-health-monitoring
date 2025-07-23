
# 🩺 IoT Stethoscope for Basic Health Monitoring

An ESP32-based smart health monitoring system that uses the MAX30105 sensor to measure **Heart Rate (BPM)**, **SpO₂ (Blood Oxygen Saturation)**, and **Body Temperature**, with real-time updates and alerts via the **Blynk IoT Platform**.

---

## 📦 Features

- 💓 Real-time Heart Rate Monitoring
- 🌬 Accurate SpO₂ (% Oxygen Saturation) Detection
- 🌡 Internal Body Temperature Monitoring (°F)
- 📲 Data Display on Blynk Mobile App
- ⚠️ Automatic Alerts for Abnormal Vitals

---

## 🧰 Hardware Required

| Component           | Description                          |
|--------------------|--------------------------------------|
| ESP32 Dev Module   | Wi-Fi-enabled microcontroller         |
| MAX30105 Sensor    | Pulse oximeter and heart rate sensor |
| Jumper Wires       | For connections                      |
| Breadboard (optional) | For prototyping                   |
| USB Cable          | To connect ESP32 to PC               |

---

## 🔌 Sensor Connections (MAX30105 to ESP32)

| MAX30105 Pin | ESP32 Pin |
|--------------|-----------|
| VIN          | 3.3V      |
| GND          | GND       |
| SDA          | GPIO 21   |
| SCL          | GPIO 22   |

---

## 🛠 Software Requirements

- **Arduino IDE** (with ESP32 board support installed)
- **Libraries to Install** via Library Manager:
  - `MAX30105` by SparkFun
  - `heartRate.h`, `spo2_algorithm.h` (part of MAX3010x library or custom)
  - `Blynk` and `BlynkSimpleEsp32.h`
- **Blynk IoT Account** and Template

---

## 📲 Blynk Configuration

1. Go to [https://blynk.cloud](https://blynk.cloud)
2. Create a new Template:
   - Template ID: `Your_Template_ID`
   - Template Name: `IOT stethescope for basic health monitoring`
3. Add a new device using the above template.
4. Add the following widgets and assign Virtual Pins:
   - **V0** → Heart Rate (Gauge or Display)
   - **V1** → SpO₂ (Gauge or Display)
   - **V2** → Temperature (Gauge or Display)
5. Add Events in Blynk:
   - `heart_rate_alert`
   - `spo2_alert`
   - `temp_alert`

---

## 🔐 Code Snippet

Replace the following placeholders in the code with your Blynk template details:

```cpp
#define BLYNK_TEMPLATE_ID "Your_Template_ID"
#define BLYNK_TEMPLATE_NAME "IOT stethescope for basic health monitoring"
#define BLYNK_AUTH_TOKEN "Your_Auth_Token"
```

---

## 📈 How It Works

- The **MAX30105** sensor uses IR and RED LEDs to measure reflectance from your finger.
- The **heart rate and SpO₂** are calculated using the **MAXIM algorithm**.
- The ESP32 also reads **internal temperature**.
- All readings are sent to **Blynk Cloud** every 5 seconds.
- Alerts are sent if abnormal values are detected.

---

## 🧪 Sample Output

```
Heart Rate: 78 bpm, SpO2: 97 %, Temp: 98.6 °F
Heart Rate: Invalid, SpO2: Invalid, Temp: 98.4 °F
```

---

## 🚨 Alert Conditions

- **Heart Rate** < 50 bpm or > 120 bpm → Triggers `heart_rate_alert`
- **SpO₂** < 92% → Triggers `spo2_alert`
- **Temperature** > 100.4 °F (≈ 38 °C) → Triggers `temp_alert`

---

## 🤳 Mobile App Preview

📲 Displays:
- Heart Rate (bpm)
- SpO₂ (%)
- Temperature (°F)

🔔 Alerts:
- Instant notification if any value crosses safe thresholds

---

## 📚 Resources

- [MAX30105 Datasheet](https://datasheets.maximintegrated.com/en/ds/MAX30105.pdf)
- [SparkFun MAX3010x Library](https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library)
- [Blynk Cloud](https://blynk.cloud)

---

## 👩‍💻 Developed By

Team Virus Vixens

---

## 🛡 License

This project is open-source and available under the [MIT License](LICENSE).
