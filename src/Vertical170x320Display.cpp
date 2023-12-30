#include "icons.h" // Include the icons header file
#include "Vertical170x320Display.h"
#include <Adafruit_gfx.h>
#include "network.h"

bool wifiStatus = true;

uint16_t determineUpperColor(int co2Value)
{
    if (co2Value == 0)
    {
        return TFT_WHITE;
    }
    else if (co2Value < 800)
    {
        return TFT_GREEN;
    }
    else if (co2Value < 1000)
    {
        return TFT_YELLOW;
    }
    else
    {
        return TFT_RED;
    }
}

void Vertical170x320Display::begin()
{
    tft.begin();
    tft.setRotation(2);
    tft.fillScreen(TFT_BLACK);

    // Create sprites for the different parts of the display
    co2Sprite.setColorDepth(8);
    co2Sprite.createSprite(tft.width(), tft.height() / 2);
    co2Sprite.setTextDatum(TC_DATUM);

    temperatureSprite.setColorDepth(8);
    temperatureSprite.createSprite(90, 30);
    temperatureSprite.setTextDatum(TC_DATUM);

    humiditySprite.setColorDepth(8);
    humiditySprite.createSprite(90, 30);
    humiditySprite.setTextDatum(TC_DATUM);
}

void Vertical170x320Display::showErrorScreen(String message)
{
    tft.setTextDatum(TC_DATUM);
    uint8_t margin = (tft.width() - 128) / 2;
    tft.drawBitmap(margin, margin, icon_error, 128, 128, TFT_RED, TFT_BLACK);

    tft.setFreeFont(&FreeSansBold9pt7b);
    tft.drawString(message, tft.width() / 2, tft.height() / 2 + 50);
    for (;;) // Endless loop to prevent further execution
    {
    }
}

void Vertical170x320Display::drawStatic()
{
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true); // Set text color and background color
    tft.setFreeFont(&FreeSans9pt7b);
    tft.drawString("Temperatur", tft.width() / 2, tft.height() / 2 + 40 - 25);

    tft.setTextColor(TFT_WHITE, TFT_BLACK, true); // Set text color and background color
    tft.setFreeFont(&FreeSans9pt7b);
    tft.drawString("Luftfeuchte", tft.width() / 2, tft.height() - 40 - 25);
}

void Vertical170x320Display::drawCo2()
{
    static uint16_t lastCo2Value = 65535;       // 65535 is the maximum value for uint16_t
    static uint16_t lastUpperColor = TFT_BLACK; // TFT_BLACK is the default background color
    static String lastCo2String = "";
    uint16_t co2Reading = sensor->getCo2Value();
    uint16_t bg_color = determineUpperColor(co2Reading);
    bool spriteUpdated = false;
    co2Sprite.setTextDatum(TC_DATUM);

    if (lastUpperColor != bg_color)
    {
        co2Sprite.fillRect(0, 0, tft.width(), tft.height() / 2, bg_color);
        co2Sprite.setTextColor(TFT_BLACK, bg_color, true);
        co2Sprite.setFreeFont(&FreeSansBold12pt7b);
        co2Sprite.drawString("ppm", tft.width() / 2, tft.height() / 6 + 50);
        lastUpperColor = bg_color;
        spriteUpdated = true;
    }
    if (lastCo2Value != co2Reading)
    {
        co2Sprite.setTextColor(TFT_BLACK, bg_color);
        co2Sprite.setFreeFont(&FreeMonoBold24pt7b);
        String co2String = "----";
        if (co2Reading > 0)
        {
            co2String = String(co2Reading);
        }
        // Overwrite the old value
        uint16_t maxTextWidth = co2Sprite.textWidth(lastCo2String);
        co2Sprite.fillRect((tft.width() / 2) - (maxTextWidth / 2), tft.height() / 6, maxTextWidth, co2Sprite.fontHeight(), bg_color);
        co2Sprite.drawString(co2String, tft.width() / 2, tft.height() / 6);
        lastCo2Value = co2Reading;
        lastCo2String = co2String;
        spriteUpdated = true;
    }

    // Push the sprite to the TFT screen
    if (spriteUpdated)
    {
        co2Sprite.pushSprite(0, 0);
    }
}

void Vertical170x320Display::drawWifiStatus()
{
    uint8_t lastWifiStatus = 255;
    static uint16_t lastUpperColor = TFT_BLACK; // TFT_BLACK is the default background color
    uint16_t co2Reading = sensor->getCo2Value();
    uint16_t upper_color = determineUpperColor(co2Reading);
    co2Sprite.setTextDatum(TL_DATUM);
    co2Sprite.setTextColor(TFT_BLACK, upper_color, true);
    wl_status_t wifiStatus = WiFi.status();

    // Clear the sprite area
    co2Sprite.fillRect(0, 0, co2Sprite.width(), 16, upper_color);
    co2Sprite.drawBitmap(tft.width() - 16, 0, icon_wifi, 16, 16, TFT_BLACK, upper_color);
    if (wifiStatus != WL_CONNECTED)
    {
        co2Sprite.drawWideLine(tft.width() - 2, 2, tft.width() - 14, 14, 2, TFT_BLACK);
        co2Sprite.drawString(translateWiFiStatus(wifiStatus), 5, 0, 2);
    }

    if (lastWifiStatus != wifiStatus || lastUpperColor != upper_color)
    {
        co2Sprite.pushSprite(0, 0);
    }
    
    lastWifiStatus = (uint8_t)wifiStatus;
    lastUpperColor = upper_color;
}

void Vertical170x320Display::drawTemperature()
{
    static uint16_t lastTemperatureReading = 65535; // 65535 is the maximum value for uint16_t
    uint16_t temperatureReading = sensor->getTemperature();

    if (lastTemperatureReading != temperatureReading)
        if (lastTemperatureReading != temperatureReading)
        {
            temperatureSprite.setTextColor(TFT_WHITE, TFT_BLACK, true); // Set text color and background color
            temperatureSprite.setFreeFont(&FreeSansBold12pt7b);
            String temperatureString = String(temperatureReading / 100.0, 1);

            // Clear the sprite area
            temperatureSprite.fillScreen(TFT_BLACK);

            // Draw new temperature value
            temperatureSprite.drawString(temperatureString + " C", temperatureSprite.width() / 2, 5);

            // Draw the degree symbol (small circle) on the sprite
            temperatureSprite.fillCircle(temperatureSprite.width() / 2 + temperatureSprite.textWidth(temperatureString) / 2 - 7, 7, 2, TFT_WHITE);

            // Push the sprite to the display
            temperatureSprite.pushSprite((tft.width() - temperatureSprite.width()) / 2, tft.height() / 2 + 40);

            // Update last temperature
            lastTemperatureReading = temperatureReading;
        }
}

void Vertical170x320Display::drawHumidity()
{
    static uint16_t lastHumidityReading = 65535; // 65535 is the maximum value for uint16_t
    uint16_t humidityReading = sensor->getHumidity();

    if (lastHumidityReading != humidityReading)
    {
        humiditySprite.setTextColor(TFT_WHITE, TFT_BLACK, true); // Set text color and background color
        humiditySprite.setFreeFont(&FreeSansBold12pt7b);
        String humidityString = String(humidityReading / 100.0, 1) + "%";

        // Clear the sprite area
        humiditySprite.fillScreen(TFT_BLACK);

        // Draw new humidity value
        humiditySprite.drawString(humidityString, humiditySprite.width() / 2, 5);

        // Push the sprite to the display
        humiditySprite.pushSprite((tft.width() - humiditySprite.width()) / 2, tft.height() - 40);

        // Update last humidity reading
        lastHumidityReading = humidityReading;
    }
}

void Vertical170x320Display::updateDisplay()
{
    static bool staticDrawn = false;
    if (sensor->hasError())
    {
        showErrorScreen("Sensor Fehler!");
        staticDrawn = false;
        return;
    }
    if (!staticDrawn)
    {
        drawStatic();
        staticDrawn = true;
    }
    drawCo2();
    drawWifiStatus();
    drawTemperature();
    drawHumidity();
}
