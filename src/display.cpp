#include "icons.h" // Include the icons header file
#include "Vertical170x320Display.h"
#include <Adafruit_gfx.h>
#include <Preferences.h>
#include "network.h"
#include "abl.h"
#include "menu.h"

static Preferences preferences;

uint16_t Vertical170x320Display::determineUpperColor(int co2Value)
{
    if (style == STYLE_NO_UPPER_COLOR)
    {
        return TFT_BLACK;
    }

    if (co2Value == 0)
    {
        return TFT_WHITE;
    }
    else if (co2Value < 900)
    {
        return TFT_DARKGREEN;
    }
    else if (co2Value < 1200)
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
    preferences.begin("display_config", false);
    abcEnabled = preferences.getBool("abcEnabled", true);
    style = STYLE_DEFAULT;
    tft.begin();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);
    screen.createSprite(tft.width(), tft.height());
    screen.fillScreen(TFT_BLACK);
    screen.setColorDepth(8);
}

void Vertical170x320Display::setAbcEnabled(bool enabled)
{
    preferences.putBool("abcEnabled", enabled);
    abcEnabled = enabled;
}

void Vertical170x320Display::setStyle(uint8_t style)
{
    if (style >= STYLE_END)
    {
        style = STYLE_DEFAULT;
    }
    this->style = style;
}

void Vertical170x320Display::showErrorScreen(String message)
{
    uint8_t margin = (screen.width() - 128) / 2;
    screen.drawBitmap(margin, margin, icon_error, 128, 128, TFT_RED, TFT_BLACK);

    screen.setFreeFont(&FreeSansBold9pt7b);
    screen.setTextDatum(TC_DATUM);
    screen.drawString(message, screen.width() / 2, screen.height() / 2 + 50);
}

void Vertical170x320Display::drawStatic()
{
    screen.setTextColor(nightMode ? TFT_RED : TFT_WHITE, TFT_BLACK, true); // Set text color and background color
    screen.setFreeFont(&FreeSans9pt7b);
    screen.setTextDatum(TC_DATUM);
    screen.drawString("Temperatur", screen.width() / 2, screen.height() / 2 + 40 - 27);
    screen.drawString("Luftfeuchte", screen.width() / 2, screen.height() - 40 - 27);
}

void Vertical170x320Display::setBrightness(uint8_t brightness)
{
    if (brightness > nightModeThreshold)
    {
        nightMode = false;
    }
    else
    {
        nightMode = true;
    }
    this->brightness = brightness;
    ledcWrite(0, brightness);
}

void Vertical170x320Display::drawCo2()
{
    uint16_t co2Reading = sensor->getCo2Value();
    uint16_t upper_color = determineUpperColor(co2Reading);
    uint16_t back_color = nightMode ? TFT_BLACK : upper_color;
    uint16_t front_color = nightMode ? TFT_RED : (upper_color != TFT_BLACK ? TFT_BLACK : TFT_WHITE);
    screen.setTextDatum(TC_DATUM);
    screen.fillRect(0, 0, screen.width(), screen.height() / 2, back_color);
    screen.setTextColor(front_color, back_color, true);
    screen.setFreeFont(&FreeSansBold12pt7b);
    screen.drawString("ppm", screen.width() / 2, screen.height() / 6 + 50);
    String co2String = "----";
    if (co2Reading > 0)
    {
        co2String = String(co2Reading);
    }
    screen.setFreeFont(&FreeSansBold24pt7b);
    screen.drawString(co2String, screen.width() / 2, screen.height() / 6);
}

void Vertical170x320Display::drawWifiStatus()
{
    uint16_t co2Reading = sensor->getCo2Value();
    uint16_t upper_color = determineUpperColor(co2Reading);
    uint16_t back_color = nightMode ? TFT_BLACK : upper_color;
    uint16_t front_color = nightMode ? TFT_RED : (upper_color != TFT_BLACK ? TFT_BLACK : TFT_WHITE);
    screen.setTextDatum(TL_DATUM);
    wl_status_t wifiStatus = WiFi.status();
    screen.drawBitmap(screen.width() - 16, 0, icon_wifi, 16, 16, front_color, back_color);
    if (wifiStatus != WL_CONNECTED)
    {
        screen.drawWideLine(screen.width() - 2, 2, screen.width() - 14, 14, 2, TFT_BLACK);
        // screen.drawString(translateWiFiStatus(wifiStatus), 5, 0, 2);
    }
}

void Vertical170x320Display::drawTemperature()
{
    uint16_t temperatureReading = sensor->getTemperature();
    screen.setTextDatum(TC_DATUM);
    screen.setTextColor(nightMode ? TFT_RED : TFT_WHITE, TFT_BLACK, true); // Set text color and background color
    screen.setFreeFont(&FreeSansBold18pt7b);
    String temperatureString = String(temperatureReading / 100.0, 1);
    // Draw new temperature value
    screen.drawString(temperatureString + " C", screen.width() / 2, screen.height() / 2 + 40);
    // Draw the degree symbol (small circle) on the sprite
    screen.fillCircle(screen.width() / 2 + screen.textWidth(temperatureString) / 2 - 9, screen.height() / 2 + 42, 2, nightMode ? TFT_RED : TFT_WHITE);
}

void Vertical170x320Display::drawHumidity()
{
    uint16_t humidityReading = sensor->getHumidity();
    screen.setTextDatum(TC_DATUM);
    screen.setTextColor(nightMode ? TFT_RED : TFT_WHITE, TFT_BLACK, true); // Set text color and background color
    screen.setFreeFont(&FreeSansBold18pt7b);
    String humidityString = String(humidityReading / 100.0, 0) + "%";
    // Draw new humidity value
    screen.drawString(humidityString, screen.width() / 2, screen.height() - 40);
}

void Vertical170x320Display::updateDisplay()
{
    setBrightness(abcEnabled ? getAblBrightness() : 255); // Set the brightness according to the ambient light sensor or to 255 if the ABC is disabled
    screen.fillScreen(TFT_BLACK);                         // Clear the sprite
    if (sensor->hasError())
    {
        showErrorScreen("Sensor ERROR!");
    }
    else
    {
        menu.handleButtons();
        if (menu.shouldShow()) // If the menu should be shown, show it
        {
            menu.show(&screen);
        }
        else // Otherwise show the normal display
        {
            drawCo2();
            if (wifiEnabled)
            {
                drawWifiStatus();
            }
            drawStatic();
            drawTemperature();
            drawHumidity();
        }
    }

    screen.pushSprite(0, 0); // Push the sprite to the display
}
