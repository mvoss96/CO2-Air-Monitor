#include <Arduino.h>
#include "Sensor.h"
#include "Vertical170x320Display.h"
#include "network.h"

Sensor sensor;
Vertical170x320Display display(&sensor);

void setup()
{
  Serial.begin(115200);
  display.begin();
  sensor.begin();
  pinMode(5, INPUT_PULLUP);
  display.updateDisplay(); // Show initial screen before connecting to WiFi
  wifiSetup();
  ledcSetup(0, 5000, 8); // Channel 0, 5000 Hz frequency, 8-bit resolution for Backlight LED
  //ledcAttachPin(TFT_BL, 0);
  //digitalWrite(TFT_BL, HIGH);
}

void loop()
{
  sensor.update();
  display.updateDisplay();
  wifiLoop(&sensor);
}
