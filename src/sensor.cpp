#include "SparkFun_SCD4x_Arduino_Library.h"
#include "sensor.h"

SCD4x mySensor;

Sensor::Sensor()
{
}

bool Sensor::begin()
{
    Wire.begin();                             // Start the I2C bus
    if (mySensor.begin(true, false) == false) // Begin measurement mode and disable automatic self-calibration
    {
        Serial.println("Sensor not detected. Please check wiring.");
        sensorError = true;
        return false;
    }
    return true;
}

bool Sensor::update()
{
    if (mySensor.readMeasurement() and !sensorError)
    {
        co2Value = mySensor.getCO2();
        temperature = mySensor.getTemperature() * 100;
        humidity = mySensor.getHumidity() * 100;
        return true;
    }
    return false;
}

bool Sensor::hasError()
{
    return sensorError;
}

uint16_t Sensor::getCo2Value()
{
    return co2Value;
}

uint16_t Sensor::getTemperature()
{
    return temperature;
}

uint16_t Sensor::getHumidity()
{
    return humidity;
}
