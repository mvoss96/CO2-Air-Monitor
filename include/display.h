#pragma once
#include <TFT_eSPI.h> // Include the TFT_eSPI library
#include "sensor.h"   // Include the sensor header file

class AbstractDisplay
{
protected:
    TFT_eSPI tft;
    Sensor *sensor;
    bool abcEnabled;

public:
    AbstractDisplay(Sensor *s) : tft(TFT_eSPI()), sensor(s) {} // Initialize the display and sensor
    virtual void begin() = 0;
    virtual void updateDisplay() = 0;
};
