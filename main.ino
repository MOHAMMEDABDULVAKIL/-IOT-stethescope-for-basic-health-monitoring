#define BLYNK_TEMPLATE_ID "BLYNK TEMPLATE ID"
#define BLYNK_TEMPLATE_NAME "IOT stethescope for basic health monitoring"
#define BLYNK_AUTH_TOKEN "BLYNK_AUTH_TOKEN"

#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include "spo2_algorithm.h"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "suri";
char pass[] = "123456789";

MAX30105 particleSensor;

const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg;

uint32_t irBuffer[100];
uint32_t redBuffer[100];
int bufferLength = 100;

int32_t spo2;
int8_t validSPO2;
int32_t heartRate;
int8_t validHeartRate;

void setup() {
  Serial.begin(115200);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("Initializing MAX30105...");
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 not found. Check wiring.");
    while (1);
  }

  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x1F);
  particleSensor.setPulseAmplitudeIR(0x1F);

  Serial.println("Place your finger on the sensor.");
}

void loop() {
  Blynk.run();

  for (int i = 0; i < bufferLength; i++) {
    while (!particleSensor.available()) {
      particleSensor.check();
    }
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();
  }

  // Calculate SpO2 and Heart Rate
  maxim_heart_rate_and_oxygen_saturation(
    irBuffer, bufferLength, redBuffer,
    &spo2, &validSPO2,
    &heartRate, &validHeartRate
  );

  // Read internal temperature in °C and convert to °F
  float tempC = particleSensor.readTemperature();
  float tempF = (tempC * 1.8) + 32.0;

  // Print to Serial
  Serial.print("Heart Rate: ");
  Serial.print(validHeartRate ? String(heartRate) + " bpm" : "Invalid");

  Serial.print(", SpO2: ");
  Serial.print(validSPO2 ? String(spo2) + " %" : "Invalid");

  Serial.print(", Temp: ");
  Serial.print(tempF);
  Serial.println(" °F");

  // Send to Blynk
  if (validHeartRate) {
    Blynk.virtualWrite(V0, heartRate);
    if (heartRate < 50 || heartRate > 120) {
      Blynk.logEvent("heart_rate_alert", "Heart Rate Abnormal: " + String(heartRate) + " bpm");
    }
  }

  if (validSPO2) {
    Blynk.virtualWrite(V1, spo2);
    if (spo2 < 92) {
      Blynk.logEvent("spo2_alert", "SpO₂ Low: " + String(spo2) + "%");
    }
  }

  // Send Fahrenheit to V2 instead of Celsius
  Blynk.virtualWrite(V2, tempF);
  if (tempF > 100.4) {  // 38°C ≈ 100.4°F
    Blynk.logEvent("temp_alert", "Body Temp High: " + String(tempF) + " °F");
  }

  delay(5000);  // Delay between readings
}
