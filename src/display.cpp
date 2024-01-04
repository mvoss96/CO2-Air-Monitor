#include "icons.h" // Include the icons header file
#include "Vertical170x320Display.h"
#include <Adafruit_gfx.h>
#include <Preferences.h>
#include "network.h"

Preferences preferences;

inline String boolToStr(bool value)
{
    return value ? "ON" : "OFF";
}

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
    preferences.begin("display_config", false);
    abcEnabled = preferences.getBool("abcEnabled", true);
    tft.begin();
    tft.setRotation(2);
    screen.createSprite(tft.width(), tft.height());
    screen.setColorDepth(8);
    screen.fillScreen(TFT_BLACK);
}

void Vertical170x320Display::showErrorScreen(String message)
{
    uint8_t margin = (screen.width() - 128) / 2;
    screen.drawBitmap(margin, margin, icon_error, 128, 128, TFT_RED, TFT_BLACK);

    screen.setFreeFont(&FreeSansBold9pt7b);
    screen.drawString(message, screen.width() / 2, screen.height() / 2 + 50);
}

void Vertical170x320Display::drawMenuItem(const String &itemName, bool condition, const String &itemValue, int &currentHeight, const uint8_t itemHeight, const uint8_t margin)
{
    screen.setTextDatum(TL_DATUM);
    screen.drawRect(0, currentHeight - 5, screen.width(), itemHeight, (condition ? TFT_WHITE : TFT_BLACK));
    screen.drawString(itemName, margin, currentHeight);
    screen.setTextDatum(TR_DATUM);
    screen.drawString(itemValue, screen.width() - margin, currentHeight);
    currentHeight += itemHeight;
}

void Vertical170x320Display::showMenuScreen()
{
    screen.setTextDatum(TC_DATUM);
    screen.setFreeFont(&FreeSansBold12pt7b);
    const uint8_t margin = 5;
    screen.setTextColor(TFT_WHITE, TFT_BLACK, true);
    screen.drawString("-- Menu --", screen.width() / 2, margin);
    screen.setFreeFont(&FreeSansBold9pt7b);
    // screen.fillRect(0, 30, screen.width(), screen.height() - 30, TFT_WHITE);
    // screen.setTextColor(TFT_BLACK, TFT_WHITE, true);

    const uint8_t itemHeight = 25;
    int currentHeight = 40;

    // Drawing menu items
    drawMenuItem("WiFi", menuIndex == MENU_WIFI_ENABLED, boolToStr(wifiEnabled), currentHeight, itemHeight, margin);
    drawMenuItem("MQTT", menuIndex == MENU_MQTT_ENABLED, boolToStr(mqttEnabled), currentHeight, itemHeight, margin);
    drawMenuItem("ABC", menuIndex == MENU_ABC_ENABLED, boolToStr(abcEnabled), currentHeight, itemHeight, margin);
    drawMenuItem("Reset WiFi", menuIndex == MENU_WIFI_RESET, "", currentHeight, itemHeight, margin);
    drawMenuItem("Exit Menu", menuIndex == MENU_EXIT, "", currentHeight, itemHeight, margin);

    // Show information about Wifi and MQTT connection
    const uint8_t infoItemHeight = 15;
    currentHeight += itemHeight; // Adjust height after menu items
    screen.setTextColor(TFT_DARKGREY, TFT_BLACK, true);
    screen.setTextDatum(TL_DATUM);
    screen.setTextFont(1);
    String infoText[] = {
        "WiFi Status: " + translateWiFiStatus(WiFi.status()),
        "IP: " + WiFi.localIP().toString(),
        "SSID: " + WiFi.SSID(),
        "MQTT Server: " + String(mqtt_server),
        "MQTT Port: " + String(mqtt_port),
        "MQTT Topic: " + String(mqtt_topic),
        "MQTT Username: " + String(mqtt_username),
        "MQTT Password: " + String(mqtt_password)};

    for (String text : infoText)
    {
        screen.drawString(text, margin, currentHeight);
        currentHeight += infoItemHeight;
    }
}

void Vertical170x320Display::drawStatic()
{
    screen.setTextDatum(TC_DATUM);
    screen.setTextColor(TFT_WHITE, TFT_BLACK, true); // Set text color and background color
    screen.setFreeFont(&FreeSans9pt7b);
    screen.drawString("Temperatur", screen.width() / 2, screen.height() / 2 + 40 - 25);
    screen.setTextColor(TFT_WHITE, TFT_BLACK, true); // Set text color and background color
    screen.setFreeFont(&FreeSans9pt7b);
    screen.drawString("Luftfeuchte", screen.width() / 2, screen.height() - 40 - 25);
}

void Vertical170x320Display::handleMenu()
{
    static unsigned long pressStartTime = 0;
    const unsigned long longPressDuration = 500;

    if (wasClicked == false && digitalRead(PIN_BTN1) == LOW)
    {
        pressStartTime = millis();
        wasClicked = true;
    }
    else if (wasClicked == true && digitalRead(PIN_BTN1) == HIGH)
    {
        wasClicked = false;
        if (menuClickWasHandled == false)
        {
            menuClickWasHandled = true;
            return;
        }
        unsigned long pressDuration = millis() - pressStartTime;
        Serial.println("Press duration: " + String(pressDuration));
        if (pressDuration >= longPressDuration)
        {
            if (menuIndex == MENU_WIFI_ENABLED)
            {
                setWifiEnabled(!wifiEnabled);
            }
            else if (menuIndex == MENU_MQTT_ENABLED)
            {
                setMqttEnabled(!mqttEnabled);
            }
            else if (menuIndex == MENU_ABC_ENABLED)
            {
                abcEnabled = !abcEnabled;
                preferences.putBool("abcEnabled", abcEnabled);
            }
            else if (menuIndex == MENU_WIFI_RESET)
            {
                wifiReset();
            }
            else if (menuIndex == MENU_EXIT)
            {
                menuActive = false;
            }
        }
        else if (++menuIndex >= MENU_END)
        {
            menuIndex = 0;
        }
    }
}

void Vertical170x320Display::drawCo2()
{
    uint16_t co2Reading = sensor->getCo2Value();
    uint16_t bg_color = determineUpperColor(co2Reading);
    screen.setTextDatum(TC_DATUM);
    screen.fillRect(0, 0, screen.width(), screen.height() / 2, bg_color);
    screen.setTextColor(TFT_BLACK, bg_color, true);
    screen.setFreeFont(&FreeSansBold12pt7b);
    screen.drawString("ppm", screen.width() / 2, screen.height() / 6 + 50);
    screen.setFreeFont(&FreeMonoBold24pt7b);
    String co2String = "----";
    if (co2Reading > 0)
    {
        co2String = String(co2Reading);
    }
    screen.drawString(co2String, screen.width() / 2, screen.height() / 6);
}

void Vertical170x320Display::drawWifiStatus()
{
    if (!wifiEnabled)
    {
        return; // Do not draw anything if wifi is disabled
    }

    uint16_t co2Reading = sensor->getCo2Value();
    uint16_t upper_color = determineUpperColor(co2Reading);
    screen.setTextDatum(TL_DATUM);
    screen.setTextColor(TFT_BLACK, upper_color, true);
    wl_status_t wifiStatus = WiFi.status();

    // Clear the sprite area
    screen.drawBitmap(screen.width() - 16, 0, icon_wifi, 16, 16, TFT_BLACK, upper_color);
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
    screen.setTextColor(TFT_WHITE, TFT_BLACK, true); // Set text color and background color
    screen.setFreeFont(&FreeSansBold12pt7b);
    String temperatureString = String(temperatureReading / 100.0, 1);
    // Draw new temperature value
    screen.drawString(temperatureString + " C", screen.width() / 2, screen.height() / 2 + 40);
    // Draw the degree symbol (small circle) on the sprite
    screen.fillCircle(screen.width() / 2 + screen.textWidth(temperatureString) / 2 - 7, screen.height() / 2 + 42, 2, TFT_WHITE);
}

void Vertical170x320Display::drawHumidity()
{
    uint16_t humidityReading = sensor->getHumidity();
    screen.setTextDatum(TC_DATUM);
    screen.setTextColor(TFT_WHITE, TFT_BLACK, true); // Set text color and background color
    screen.setFreeFont(&FreeSansBold12pt7b);
    String humidityString = String(humidityReading / 100.0, 1) + "%";
    // Draw new humidity value
    screen.drawString(humidityString, screen.width() / 2, screen.height() - 40);
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
        if (digitalRead(PIN_BTN1) == LOW && !menuActive)
        {
            wasClicked = true;
            menuClickWasHandled = false;
            menuActive = true;
        }

        if (menuActive)
        {
            handleMenu();
            showMenuScreen();
        }
        else
        {
            drawStatic();
            drawCo2();
            drawWifiStatus();
            drawTemperature();
            drawHumidity();
        }
    }

    screen.pushSprite(0, 0); // Push the sprite to the display
}
