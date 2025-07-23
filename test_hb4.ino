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

int irThresholdLow = 50000;      // too low = no finger
int irThresholdHigh = 200000;    // too high = saturation
byte irLedPower = 0x20;          // starting value (adjusted dynamically)

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);  // ESP32 I2C pins

  if (!sensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 not found. Check wiring.");
    while (1);
  }

  sensor.setup();  // default settings
  sensor.setPulseAmplitudeRed(0x1F); // red not used for pulse detection
  sensor.setPulseAmplitudeIR(irLedPower);
  sensor.setPulseAmplitudeGreen(0); // turn off green

  Serial.println("Place your finger gently on the sensor.");
}

void loop() {
  long irValue = sensor.getIR();

  // Auto-adjust IR LED brightness if saturated
  if (irValue >= 262143) {
    irLedPower = max(irLedPower - 1, 0x01);  // reduce intensity
    sensor.setPulseAmplitudeIR(irLedPower);
    Serial.println("Saturation detected. Lowering IR power.");
  } else if (irValue < irThresholdLow) {
    irLedPower = min(irLedPower + 1, 0xFF);  // boost if too low
    sensor.setPulseAmplitudeIR(irLedPower);
    Serial.println("Low IR signal. Increasing IR power.");
  }

  // Only try beat detection if signal is in a good range
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

        Serial.print("IR=");
        Serial.print(irValue);
        Serial.print(", BPM=");
        Serial.print(beatsPerMinute);
        Serial.print(", Avg BPM=");
        Serial.println(beatAvg);
      } else {
        Serial.println("Out of range BPM. Skipping...");
      }
    } else {
      Serial.print("IR=");
      Serial.print(irValue);
      Serial.println(" - Waiting for beat...");
    }
  } else if (irValue < irThresholdLow) {
    Serial.print("IR=");
    Serial.print(irValue);
    Serial.println(" - No finger detected.");
  } else if (irValue >= irThresholdHigh) {
    Serial.print("IR=");
    Serial.print(irValue);
    Serial.println(" - Sensor saturated.");
  }

  delay(20);
}
