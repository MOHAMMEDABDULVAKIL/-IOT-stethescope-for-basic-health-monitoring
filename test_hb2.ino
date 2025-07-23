#include <Wire.h>
#include "MAX30105.h"

MAX30105 sensor;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);  // ESP32 I2C default pins

  if (!sensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 not found. Check wiring.");
    while (1);
  }

  sensor.setup();
  sensor.setPulseAmplitudeRed(0x00);  // Off
  sensor.setPulseAmplitudeIR(0xFF);   // Max IR power
}

void loop() {
  long ir = sensor.getIR();
  Serial.print("IR: ");
  Serial.println(ir);
  delay(100);
}
