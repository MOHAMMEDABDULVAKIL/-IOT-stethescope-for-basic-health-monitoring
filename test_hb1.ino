#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

MAX30105 particleSensor;

const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;

float beatsPerMinute;
int beatAvg;

void setup() {
  Serial.begin(9600);
  Serial.println("Initializing...");

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 not found. Check wiring.");
    while (1);
  }

  particleSensor.setup();                     
  particleSensor.setPulseAmplitudeRed(0x32);  // Increase intensity
  particleSensor.setPulseAmplitudeGreen(0);   // Turn off green

  Serial.println("Place your finger flat and still.");
}

void loop() {
  long irValue = particleSensor.getIR();

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
      Serial.println("Invalid BPM reading, skipping...");
    }
  }

  if (irValue < 50000) {
    Serial.println("No finger detected or weak signal.");
  }

  delay(50);  // Small delay to stabilize loop
}
