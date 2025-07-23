#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include <WiFi.h>           // Use <ESP8266WiFi.h> if using ESP8266
#include <BlynkSimpleEsp32.h>  // Use appropriate board-specific Blynk library

char auth[] = "Your_Blynk_Auth_Token";  // Replace with your Blynk Auth Token
char ssid[] = "Your_WiFi_SSID";         // Replace with your WiFi SSID
char pass[] = "Your_WiFi_Password";     // Replace with your WiFi password

MAX30105 particleSensor;
#define MAX_BRIGHTNESS 255

uint32_t irBuffer[100];
uint32_t redBuffer[100];

int32_t bufferLength;
int32_t spo2;
int8_t validSPO2;
int32_t heartRate;
int8_t validHeartRate;

byte pulseLED = 11;
byte readLED = 13;

void setup()
{
  Serial.begin(115200);
  pinMode(pulseLED, OUTPUT);
  pinMode(readLED, OUTPUT);

  Blynk.begin(auth, ssid, pass);  // Connect to Blynk

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST))
  {
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));
    while (1);
  }

  Serial.println(F("Attach sensor to finger with rubber band. Press any key to start conversion"));
  while (Serial.available() == 0) ;
  Serial.read();

  byte ledBrightness = 60;
  byte sampleAverage = 4;
  byte ledMode = 2;
  byte sampleRate = 100;
  int pulseWidth = 411;
  int adcRange = 4096;

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
}

void loop()
{
  Blynk.run();  // Required to keep Blynk connection alive

  bufferLength = 100;
  for (byte i = 0; i < bufferLength; i++)
  {
    while (particleSensor.available() == false)
      particleSensor.check();

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();

    Serial.print(F("red="));
    Serial.print(redBuffer[i], DEC);
    Serial.print(F(", ir="));
    Serial.println(irBuffer[i], DEC);
  }

  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

  static int hrWindow[4] = {0};
  static byte hrIndex = 0;
  int hrAvg = 0;

  while (1)
  {
    for (byte i = 25; i < 100; i++)
    {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }

    for (byte i = 75; i < 100; i++)
    {
      while (particleSensor.available() == false)
        particleSensor.check();

      digitalWrite(readLED, !digitalRead(readLED));

      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample();

      Serial.print(F("red="));
      Serial.print(redBuffer[i], DEC);
      Serial.print(F(", ir="));
      Serial.print(irBuffer[i], DEC);

      if (validHeartRate && heartRate >= 40 && heartRate <= 180) {
        hrWindow[hrIndex++] = heartRate;
        hrIndex %= 4;

        hrAvg = 0;
        for (int j = 0; j < 4; j++) hrAvg += hrWindow[j];
        hrAvg /= 4;

        Serial.print(F(", HR="));
        Serial.print(hrAvg);
        Blynk.virtualWrite(V1, hrAvg);  // Send Average HR to V1
      } else {
        Serial.print(F(", HR=INVALID"));
      }

      Serial.print(F(", HRvalid="));
      Serial.print(validHeartRate, DEC);

      Serial.print(F(", SPO2="));
      Serial.print(spo2, DEC);
      Serial.print(F(", SPO2Valid="));
      Serial.println(validSPO2, DEC);

      if (validSPO2 && spo2 >= 80 && spo2 <= 100)
        Blynk.virtualWrite(V2, spo2);  // Send SpO2 to V2
    }

    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  }
}
