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
    // mySensor.startPeriodicMeasurement(); // Start periodic measurements in high power mode (5 seconds)
    sensorStartupTime = millis();
    return true;
}

bool Sensor::update()
{
    if (mySensor.readMeasurement() and !sensorError)
    {
        const unsigned long time = millis() - sensorStartupTime;
        co2Value = (time > startupTimeC * 1000) ? mySensor.getCO2() : 0;
        temperature = (time > startupTimeT * 1000) ? mySensor.getTemperature() * 100 : 0;
        humidity = (time > startupTimeH * 1000) ? mySensor.getHumidity() * 100 : 0;
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
    return (!sensorError &&
            (millis() - sensorStartupTime) > startupTimeC * 1000 &&
            (millis() - sensorStartupTime) > startupTimeT * 1000 &&
            (millis() - sensorStartupTime) > startupTimeH * 1000 &&
            mySensor.getDataReadyStatus());
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

void Sensor::setTemperatureOffset(int16_t offset)
{
    if (offset > maxTemperatureOffset)
    {
        offset = maxTemperatureOffset;
    }
    else if (offset < minTemperatureOffset)
    {
        offset = minTemperatureOffset;
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
    // Step through the temperature offset values 
    temperatureOffset += temperatureOffsetStep;
    if (temperatureOffset > maxTemperatureOffset)
    {
        temperatureOffset = minTemperatureOffset;
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