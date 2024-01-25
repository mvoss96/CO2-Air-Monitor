#pragma once
#include <TFT_eSPI.h> // Include the TFT_eSPI library
#include "sensor.h"   // Include the sensor header file

class AbstractDisplay
{
protected:
    TFT_eSPI tft;
    Sensor *sensor;
    bool abcEnabled;
    uint8_t brightness = 255;
    uint8_t style = 0;

public:
    AbstractDisplay(Sensor *s) : tft(TFT_eSPI()), sensor(s) {} // Initialize the display and sensor
    Sensor* getSensor() { return sensor; }
    uint8_t getStyle() { return style; }
    void setNextStyle() { setStyle(style + 1); }
    virtual uint8_t getBrightness() { return brightness; }
    virtual bool getAbcEnabled() { return abcEnabled; }
    virtual void setStyle(uint8_t style) = 0;
    virtual void setAbcEnabled(bool enabled) = 0;
    virtual void setBrightness(uint8_t brightness) = 0;
    virtual void begin() = 0;
    virtual void updateDisplay() = 0;
};
