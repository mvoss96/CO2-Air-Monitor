#pragma once
#include <Arduino.h>

class Sensor
{
private:
    uint16_t co2Value = 0;    // PPM value
    uint16_t temperature = 0; // Tempearture in C * 100
    uint16_t humidity = 0;    // Humidity in % * 100
    bool sensorError = false; // Error flag

public:
    Sensor();
    bool begin();              // Start the sensor and return true if it was detected
    bool update();             // Update the sensor values, returns true if new values are available
    bool hasError();           // Returns true if an error occured
    uint16_t getCo2Value();    // Get the CO2 value
    uint16_t getTemperature(); // Get the temperature
    uint16_t getHumidity();    // Get the humidity
};
