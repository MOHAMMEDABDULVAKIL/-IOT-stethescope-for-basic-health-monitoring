#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

MAX30105 sensor;

const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;

float beatsPerMinute;
int beatAvg;

int irThresholdLow = 50000;       // Below this → weak signal
int irThresholdHigh = 260000;     // Above this → potential saturation
byte irLedPower = 0x10;           // Initial IR LED power

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);  // ESP32 I2C

  if (!sensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 not found. Check wiring.");
    while (1);
  }

  sensor.setup();  // Default config
  sensor.setPulseAmplitudeRed(0x00);        // Red off (not used)
  sensor.setPulseAmplitudeGreen(0x00);      // Green off
  sensor.setPulseAmplitudeIR(irLedPower);   // Start with dim IR

  Serial.println("Place your finger gently on the sensor.");
}

void loop() {
  long irValue = sensor.getIR();

  // Adjust IR LED dynamically
  if (irValue >= 260000) {
    irLedPower = max(irLedPower - 10, 1);
    sensor.setPulseAmplitudeIR(irLedPower);
    Serial.print("⚠️ Sensor saturated. Reducing IR LED to 0x");
    Serial.println(irLedPower, HEX);
  } else if (irValue < irThresholdLow) {
    irLedPower = min(irLedPower + 10, 0xFF);
    sensor.setPulseAmplitudeIR(irLedPower);
    Serial.print("🔄 Low signal. Increasing IR LED to 0x");
    Serial.println(irLedPower, HEX);
  }

  // Detect heart beat only if signal is in good range
  if (irValue > irThresholdLow && irValue < irThresholdHigh) {
    if (checkForBeat(irValue)) {
      long delta = millis() - lastBeat;
      lastBeat = millis();

      beatsPerMinute = 60 / (delta / 1000.0);

      if (beatsPerMinute >= 50 && beatsPerMinute <= 130) {
        rates[rateSpot++] = (byte)beatsPerMinute;
        rateSpot %= RATE_SIZE;

        beatAvg = 0;
        for (byte x = 0; x < RATE_SIZE; x++) beatAvg += rates[x];
        beatAvg /= RATE_SIZE;

        Serial.print("❤️ IR=");
        Serial.print(irValue);
        Serial.print("  BPM=");
        Serial.print(beatsPerMinute);
        Serial.print("  Avg BPM=");
        Serial.print(beatAvg);
        Serial.print("  IR LED=0x");
        Serial.println(irLedPower, HEX);
      }
    } else {
      Serial.print("...Waiting for beat... IR=");
      Serial.print(irValue);
      Serial.print("  IR LED=0x");
      Serial.println(irLedPower, HEX);
    }
  } else {
    if (irValue < irThresholdLow) {
      Serial.print("👆 No finger detected. IR=");
      Serial.print(irValue);
    } else {
      Serial.print("🔥 Saturated. IR=");
      Serial.print(irValue);
    }
    Serial.print("  IR LED=0x");
    Serial.println(irLedPower, HEX);
  }

  delay(20);
}
