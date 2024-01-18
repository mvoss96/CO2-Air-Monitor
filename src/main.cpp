#include <Arduino.h>
#include "Sensor.h"
#include "Vertical170x320Display.h"
#include "network.h"

Sensor sensor;
Vertical170x320Display display(&sensor);

void setup()
{
  pinMode(PIN_BL, OUTPUT);
  pinMode(PIN_BTN1, INPUT_PULLUP);
  pinMode(PIN_ABL, INPUT);
  Serial.begin(115200);
  display.begin();
  ledcSetup(0, 5000, 8); // Channel 0, 5000 Hz frequency, 8-bit resolution for Backlight LED
  ledcAttachPin(PIN_BL, 0);
  sensor.begin();
  display.updateDisplay(); // Show initial screen before connecting to WiFi
  wifiSetup();
  
}

void loop()
{
  sensor.update();
  display.updateDisplay();
  wifiLoop(&sensor);
}
