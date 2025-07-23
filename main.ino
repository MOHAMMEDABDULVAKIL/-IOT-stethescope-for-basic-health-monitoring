// Replace with your actual Blynk details
#define BLYNK_TEMPLATE_ID "YourTemplateID"
#define BLYNK_TEMPLATE_NAME "Health Monitor"
#define BLYNK_AUTH_TOKEN "YourAuthToken"

#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <Adafruit_MLX90614.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "YourSSID";
char pass[] = "YourWiFiPassword";

MAX30105 particleSensor;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
BlynkTimer timer;

long lastBeat = 0;
const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
float beatsPerMinute;
int beatAvg;

void readSensors() {
  float temp = mlx.readObjectTempC();
  Blynk.virtualWrite(V0, temp);  // Send temp to Blynk

  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue)) {
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);
    rates[rateSpot++] = (byte)beatsPerMinute;
    rateSpot %= RATE_SIZE;

    beatAvg = 0;
    for (byte x = 0; x < RATE_SIZE; x++) beatAvg += rates[x];
    beatAvg /= RATE_SIZE;

    Serial.print("BPM: ");
    Serial.println(beatAvg);

    Blynk.virtualWrite(V1, beatAvg);
  } else {
    Serial.println("Waiting for heartbeat...");
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  mlx.begin();

  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("MAX30102 not found. Check wiring.");
    while (1);
  }

  particleSensor.setup(); // Use default settings
  particleSensor.setPulseAmplitudeRed(0x0A);  // Red LED
  particleSensor.setPulseAmplitudeIR(0x0A);   // IR LED

  Blynk.begin(auth, ssid, pass);
  timer.setInterval(2000L, readSensors);
}

void loop() {
  Blynk.run();
  timer.run();
}

