#include "menu.h"
#include "sensor.h"
#include "abl.h"
#include "network.h"

inline String boolToStr(bool value)
{
    return value ? "ON" : "OFF";
}

Menu::Menu(AbstractDisplay *display)
{
    this->display = display;
    this->sensor = display->getSensor();
}

void Menu::drawMenuItem(TFT_eSprite *screen, const String &itemName, bool condition, const String &itemValue, int &currentHeight, const uint8_t itemHeight, const uint8_t margin)
{
    screen->setTextDatum(TL_DATUM);
    screen->drawRect(0, currentHeight - 5, screen->width(), itemHeight, (condition ? (inMenuItem ? TFT_RED : TFT_WHITE) : TFT_BLACK));
    screen->drawString(itemName, margin, currentHeight);
    screen->setTextDatum(TR_DATUM);
    screen->drawString(itemValue, screen->width() - margin, currentHeight);
    currentHeight += itemHeight;
}

bool Menu::shouldShow()
{
    return menuActive;
}

void Menu::show(TFT_eSprite *screen)
{
    if (inInfoScreen)
    {
        showInfoScreen(screen);
        return;
    }
    const uint8_t margin = 5;
    const uint8_t itemHeight = 24;
    int currentHeight = 28;

    // Draw menu
    screen->setTextDatum(TC_DATUM);
    screen->setFreeFont(&FreeSansBold9pt7b);
    screen->setTextColor(TFT_WHITE, TFT_BLACK, true);
    screen->drawString("-- Menu --", screen->width() / 2, margin);
    screen->setFreeFont(&FreeSansBold9pt7b);

    // Drawing menu items
    drawMenuItem(screen, "Style", menuIndex == MENU_STYLE, String(display->getStyle()), currentHeight, itemHeight, margin);
    drawMenuItem(screen, "WiFi", menuIndex == MENU_WIFI_ENABLED, boolToStr(wifiEnabled), currentHeight, itemHeight, margin);
    drawMenuItem(screen, "MQTT", menuIndex == MENU_MQTT_ENABLED, boolToStr(mqttEnabled), currentHeight, itemHeight, margin);
    drawMenuItem(screen, "ABC", menuIndex == MENU_ABC_ENABLED, boolToStr(display->getAbcEnabled()), currentHeight, itemHeight, margin);
    drawMenuItem(screen, "T Offset", menuIndex == MENU_TEMP_OFFSET, String(sensor->getTemperatureOffset() / 100.0, 1), currentHeight, itemHeight, margin);
    drawMenuItem(screen, "H Offset", menuIndex == MENU_HUMID_OFFSET, String(sensor->getHumidityOffset() / 100.0, 1), currentHeight, itemHeight, margin);
    drawMenuItem(screen, "FRC Value", menuIndex == MENU_FRC_VALUE, String(sensor->getFrcValue()), currentHeight, itemHeight, margin);
    drawMenuItem(screen, "INFO", menuIndex == MENU_INFO, "", currentHeight, itemHeight, margin);
    drawMenuItem(screen, "Start FRC", menuIndex == MENU_FRC_START, "", currentHeight, itemHeight, margin);
    drawMenuItem(screen, "Reset WiFi", menuIndex == MENU_WIFI_RESET, "", currentHeight, itemHeight, margin);
    drawMenuItem(screen, "Reboot", menuIndex == MENU_REBOOT, "", currentHeight, itemHeight, margin);
    drawMenuItem(screen, "Exit Menu", menuIndex == MENU_EXIT, "", currentHeight, itemHeight, margin);
}

void Menu::showInfoScreen(TFT_eSprite *screen)
{
    // Show information about Wifi and MQTT connection
    const uint8_t infoItemHeight = 10;
    const uint8_t margin = 5;
    int currentHeight = 28;

    screen->setTextDatum(TC_DATUM);
    screen->setFreeFont(&FreeSansBold9pt7b);
    screen->setTextColor(TFT_WHITE, TFT_BLACK, true);
    screen->drawString("-- INFO --", screen->width() / 2, margin);

    screen->setTextColor(TFT_DARKGREY, TFT_BLACK, true);
    screen->setTextDatum(TL_DATUM);
    screen->setTextFont(1);
    String infoText[] = {
        "Chip ID: " + String(getChipID()),
        "Compile Date: " + String(__DATE__),
        "WiFi Status: " + translateWiFiStatus(WiFi.status()),
        "IP: " + WiFi.localIP().toString(),
        "SSID: " + WiFi.SSID(),
        "MQTT Server: " + String(mqtt_server),
        "MQTT Port: " + String(mqtt_port),
        "MQTT Topic: " + String(mqtt_topic),
        "ABL Value: " + String(getAblValue()),
        "Screen Brightness: " + String(display->getBrightness()),
    };

    for (String text : infoText)
    {
        screen->drawString(text, margin, currentHeight);
        currentHeight += infoItemHeight;
    }
}

void Menu::handleButtons()
{
    // Enter menu
    if (digitalRead(PIN_BTN1) == LOW && !menuActive)
    {
        wasClicked = true;
        menuClickWasHandled = false;
        menuActive = true;
        return;
    }

    // Handle Buttons inside menu
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
        if (inInfoScreen)
        {
            inInfoScreen = false;
            return;
        }

        unsigned long pressDuration = millis() - pressStartTime;
        // Serial.println("Press duration: " + String(pressDuration));
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
                display->setAbcEnabled(!display->getAbcEnabled()); // Toggle ABC
            }
            else if (menuIndex == MENU_WIFI_RESET)
            {
                wifiReset();
            }
            else if (menuIndex == MENU_TEMP_OFFSET || menuIndex == MENU_HUMID_OFFSET || menuIndex == MENU_FRC_VALUE)
            {
                inMenuItem = !inMenuItem;
            }
            else if (menuIndex == MENU_EXIT)
            {
                menuActive = false;
            }
            else if (menuIndex == MENU_FRC_START)
            {
                sensor->startFRC();
            }
            else if (menuIndex == MENU_INFO)
            {
                inInfoScreen = true;
            }
            else if (menuIndex == MENU_STYLE)
            {
                display->setNextStyle();
            }
            else if (menuIndex == MENU_REBOOT)
            {
                ESP.restart();
            }
        }
        else if (inMenuItem)
        {
            if (menuIndex == MENU_TEMP_OFFSET)
            {
                sensor->stepTemperatureOffset();
            }
            else if (menuIndex == MENU_HUMID_OFFSET)
            {
                sensor->stepHumidityOffset();
            }
            else if (menuIndex == MENU_FRC_VALUE)
            {
                sensor->stepFrcValue();
            }
        }
        else if (++menuIndex >= MENU_END)
        {
            menuIndex = 0;
        }
    }
}