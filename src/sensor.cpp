#include "SparkFun_SCD4x_Arduino_Library.h"
#include "sensor.h"
#include <Preferences.h>

SCD4x mySensor;
static Preferences preferences;

Sensor::Sensor()
{
}

bool Sensor::begin()
{
    Wire.begin(); // Start the I2C bus

    if (mySensor.begin(false, false) == false) // Begin measurement mode and disable automatic self-calibration
    {
        Serial.println("Sensor not detected. Please check wiring.");
        sensorError = true;
        return false;
    }

    preferences.begin("sensor_config", false);
    temperatureOffset = preferences.getShort("t_offset", mySensor.getTemperatureOffset() * 100);
    humidityOffset = preferences.getShort("h_offset", 0);
    frcValue = preferences.getShort("frc_value", minFrcValue);
    Serial.println("Sensor begin");
    Serial.print("Temperature Offset: ");
    Serial.println(temperatureOffset);
    Serial.print("Humidity Offset: ");
    Serial.println(humidityOffset);

    mySensor.setTemperatureOffset(temperatureOffset / 100);
    Serial.print(F("Temperature offset is: "));
    Serial.println(mySensor.getTemperatureOffset(), 2); // Print the temperature offset with two decimal places
    Serial.print(F("Sensor altitude is currently: "));
    Serial.println(mySensor.getSensorAltitude());
    Serial.print("Automatic Self Calibration Enabled: ");
    Serial.println(mySensor.getAutomaticSelfCalibrationEnabled());
    mySensor.startLowPowerPeriodicMeasurement(); // Start periodic measurements in low power mode (30 seconds)
    return true;
}

bool Sensor::update()
{
    if (!ready)
    {
        // Wait for the sensor to stabilize
        if (millis() > warmupTime * 1000)
        {
            ready = true;
        }
        else
        {
            return false;
        }
    }

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

bool Sensor::isReady()
{
    return ready;
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
    return humidity + humidityOffset;
}

int16_t Sensor::getTemperatureOffset()
{
    return temperatureOffset;
}

int16_t Sensor::getHumidityOffset()
{
    return humidityOffset;
}

uint16_t Sensor::getFrcValue()
{
    return frcValue;
}

uint16_t Sensor::getRemainingHeatupTime()
{
    if (millis() > warmupTime * 1000)
    {
        return 0;
    }
    return (warmupTime * 1000 - millis()) / 1000;
}

void Sensor::setTemperatureOffset(int16_t offset)
{
    if (offset > maxTemperatureOffset)
    {
        offset = maxTemperatureOffset;
    }
    else if (offset < -maxTemperatureOffset)
    {
        offset = -maxTemperatureOffset;
    }
    temperatureOffset = offset;
    preferences.putShort("t_offset", temperatureOffset);
}

void Sensor::setHumidityOffset(int16_t offset)
{
    if (offset > maxHumidityOffset)
    {
        offset = maxHumidityOffset;
    }
    else if (offset < -maxHumidityOffset)
    {
        offset = -maxHumidityOffset;
    }
    humidityOffset = offset;
    preferences.putShort("h_offset", humidityOffset);
}

void Sensor::setFrcValue(uint16_t value)
{
    if (value > maxFrcValue)
    {
        value = maxFrcValue;
    }
    else if (value < minFrcValue)
    {
        value = minFrcValue;
    }
    frcValue = value;
    preferences.putShort("frc_value", frcValue);
}

void Sensor::stepTemperatureOffset()
{
    // Step through the temperature offset values from -maxTemperatureOffset to +maxTemperatureOffset
    temperatureOffset += temperatureOffsetStep;
    if (temperatureOffset > maxTemperatureOffset)
    {
        temperatureOffset = 0;
    }
    Serial.print("Temperature Offset: ");
    Serial.println(temperatureOffset);
    preferences.putShort("t_offset", temperatureOffset);
}

void Sensor::stepHumidityOffset()
{
    // Step through the humidity offset values from -maxHumidityOffset to +maxHumidityOffset
    humidityOffset += humidityOffsetStep;
    if (humidityOffset > maxHumidityOffset)
    {
        humidityOffset = -maxHumidityOffset;
    }
    preferences.putShort("h_offset", humidityOffset);
}

void Sensor::stepFrcValue()
{
    // Step through the FRC values from minFrcValue to maxFrcValue
    frcValue += frcValueStep;
    if (frcValue > maxFrcValue)
    {
        frcValue = minFrcValue;
    }
    preferences.putShort("frc_value", frcValue);
}

void Sensor::startFRC()
{
    mySensor.stopPeriodicMeasurement();
    delay(500);
    float correction;
    Serial.print("Starting FRC with value: ");
    Serial.println(frcValue);
    mySensor.performForcedRecalibration(frcValue, &correction);
    mySensor.persistSettings();
    Serial.print("FRC completed. correction value: ");
    delay(500);
    ESP.restart();
}