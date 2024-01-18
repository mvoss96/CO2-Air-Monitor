#pragma once
#include <Arduino.h>

class Sensor
{
private:
    uint16_t co2Value = 0;                     // PPM value
    uint16_t temperature = 0;                  // Tempearture in C * 100
    const uint16_t maxTemperatureOffset = 800; // Maximum temperature offset in C * 100
    const uint16_t temperatureOffsetStep = 10; // Temperature offset step in C * 100
    const uint16_t warmupTime = 120;           // Warmup time in seconds
    int16_t temperatureOffset = 0;             // Tempearture offset in C * 100
    uint16_t humidity = 0;                     // Humidity in % * 100
    const uint16_t maxHumidityOffset = 1000;   // Maximum humidity offset in % * 100
    const uint16_t humidityOffsetStep = 100;   // Humidity offset step in % * 100
    int16_t humidityOffset = 0;                // Humidity in % * 100
    uint16_t frcValue = 0;                     // FRC value
    const uint16_t minFrcValue = 400;          // Maximum FRC value
    const uint16_t maxFrcValue = 800;          // Maximum FRC value
    const uint16_t frcValueStep = 10;          // FRC value step
    bool sensorError = false;                  // Error flag
    bool ready = false;                      // Flag to indicate that the sensor is ready

public:
    Sensor();
    bool begin();                              // Start the sensor and return true if it was detected
    bool update();                             // Update the sensor values, returns true if new values are available
    bool hasError();                           // Returns true if an error occured
    bool isReady();                            // Returns true if the sensor is ready
    uint16_t getRemainingHeatupTime();         // Get the remaining heatup time in seconds
    uint16_t getCo2Value();                    // Get the CO2 value
    uint16_t getTemperature();                 // Get the temperature
    uint16_t getHumidity();                    // Get the humidity
    uint16_t getFrcValue();                    // Get the FRC value
    int16_t getHumidityOffset();               // Get the humidity offset
    int16_t getTemperatureOffset();            // Get the temperature offset
    void stepTemperatureOffset();              // Step the temperature offset
    void setTemperatureOffset(int16_t offset); // Set the temperature offset
    void stepFrcValue();                       // Step the FRC value
    void setFrcValue(uint16_t value);          // Set the FRC value
    void stepHumidityOffset();                 // Step the humidity offset
    void setHumidityOffset(int16_t offset);    // Set the humidity offset
    void startFRC();                           // Start the forced recalibration
};
