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

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);  // ESP32 default I2C pins

  if (!sensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 not found. Check wiring.");
    while (1);
  }

  sensor.setup();
  sensor.setPulseAmplitudeRed(0x3F);  // Increase if needed
  sensor.setPulseAmplitudeIR(0xFF);   // Max IR
  sensor.setPulseAmplitudeGreen(0);   // Turn off Green

  Serial.println("Place your finger on the sensor.");
}

void loop() {
  long irValue = sensor.getIR();

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
    }
  } else {
    Serial.print("IR=");
    Serial.print(irValue);
    Serial.println(" - Waiting for beat...");
  }

  delay(20);  // Smooth sampling
}
