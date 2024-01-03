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
    screen.createSprite(170, 320);
    screen.setColorDepth(8);
    screen.fillScreen(TFT_BLACK);
}

void Vertical170x320Display::showErrorScreen(String message)
{
    uint8_t margin = (tft.width() - 128) / 2;
    screen.drawBitmap(margin, margin, icon_error, 128, 128, TFT_RED, TFT_BLACK);

    screen.setFreeFont(&FreeSansBold9pt7b);
    screen.drawString(message, tft.width() / 2, tft.height() / 2 + 50);
}

void Vertical170x320Display::drawStatic()
{
    screen.setTextDatum(TC_DATUM);
    screen.setTextColor(TFT_WHITE, TFT_BLACK, true); // Set text color and background color
    screen.setFreeFont(&FreeSans9pt7b);
    screen.drawString("Temperatur", tft.width() / 2, tft.height() / 2 + 40 - 25);
    screen.setTextColor(TFT_WHITE, TFT_BLACK, true); // Set text color and background color
    screen.setFreeFont(&FreeSans9pt7b);
    screen.drawString("Luftfeuchte", tft.width() / 2, tft.height() - 40 - 25);
}

void Vertical170x320Display::drawCo2()
{
    uint16_t co2Reading = sensor->getCo2Value();
    uint16_t bg_color = determineUpperColor(co2Reading);
    screen.setTextDatum(TC_DATUM);
    screen.fillRect(0, 0, tft.width(), tft.height() / 2, bg_color);
    screen.setTextColor(TFT_BLACK, bg_color, true);
    screen.setFreeFont(&FreeSansBold12pt7b);
    screen.drawString("ppm", tft.width() / 2, tft.height() / 6 + 50);
    screen.setFreeFont(&FreeMonoBold24pt7b);
    String co2String = "----";
    if (co2Reading > 0)
    {
        co2String = String(co2Reading);
    }
    screen.drawString(co2String, tft.width() / 2, tft.height() / 6);
}

void Vertical170x320Display::drawWifiStatus()
{
    uint16_t co2Reading = sensor->getCo2Value();
    uint16_t upper_color = determineUpperColor(co2Reading);
    screen.setTextDatum(TL_DATUM);
    screen.setTextColor(TFT_BLACK, upper_color, true);
    wl_status_t wifiStatus = WiFi.status();

    // Clear the sprite area
    screen.drawBitmap(tft.width() - 16, 0, icon_wifi, 16, 16, TFT_BLACK, upper_color);
    if (wifiStatus != WL_CONNECTED)
    {
        screen.drawWideLine(tft.width() - 2, 2, tft.width() - 14, 14, 2, TFT_BLACK);
        screen.drawString(translateWiFiStatus(wifiStatus), 5, 0, 2);
    }
}

void Vertical170x320Display::drawTemperature()
{
    uint16_t temperatureReading = sensor->getTemperature();
    screen.setTextDatum(TC_DATUM);
    screen.setTextColor(TFT_WHITE, TFT_BLACK, true); // Set text color and background color
    screen.setFreeFont(&FreeSansBold12pt7b);
    String temperatureString = String(temperatureReading / 100.0, 1);
    // Draw new temperature value
    screen.drawString(temperatureString + " C", screen.width() / 2, tft.height() / 2 + 40);
    // Draw the degree symbol (small circle) on the sprite
    screen.fillCircle(screen.width() / 2 + screen.textWidth(temperatureString) / 2 - 7, tft.height() / 2 + 42, 2, TFT_WHITE);
}

void Vertical170x320Display::drawHumidity()
{
    uint16_t humidityReading = sensor->getHumidity();
    screen.setTextDatum(TC_DATUM);
    screen.setTextColor(TFT_WHITE, TFT_BLACK, true); // Set text color and background color
    screen.setFreeFont(&FreeSansBold12pt7b);
    String humidityString = String(humidityReading / 100.0, 1) + "%";
    // Draw new humidity value
    screen.drawString(humidityString, screen.width() / 2, tft.height() - 40);
}

void Vertical170x320Display::updateDisplay()
{
    screen.fillScreen(TFT_BLACK);
    if (sensor->hasError())
    {
        showErrorScreen("Sensor Fehler!");
    }
    else
    {
        drawStatic();
        drawCo2();
        drawWifiStatus();
        drawTemperature();
        drawHumidity();
    }
    screen.pushSprite(0, 0); // Push the sprite to the display
}
