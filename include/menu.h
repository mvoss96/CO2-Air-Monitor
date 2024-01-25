#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h> // Include the TFT_eSPI library
#include "display.h"

enum menuItems : uint8_t
{
    MENU_STYLE,
    MENU_WIFI_ENABLED,
    MENU_MQTT_ENABLED,
    MENU_ABC_ENABLED,
    MENU_TEMP_OFFSET,
    MENU_HUMID_OFFSET,
    MENU_FRC_VALUE,
    MENU_INFO,
    MENU_FRC_START,
    MENU_WIFI_RESET,
    MENU_REBOOT,
    MENU_EXIT,
    MENU_END,
};

class Menu
{
private:
    bool menuActive = false;
    bool wasClicked = false;
    bool menuClickWasHandled = false;
    bool inMenuItem = false;
    bool inInfoScreen = false;
    uint8_t menuIndex = MENU_EXIT;
    AbstractDisplay *display;
    Sensor *sensor;
    void showInfoScreen(TFT_eSprite *screen);
    void drawMenuItem(TFT_eSprite *screen, const String &itemName, bool condition, const String &itemValue, int &currentHeight, const uint8_t itemHeight, const uint8_t margin);

public:
    Menu(AbstractDisplay *display);
    bool shouldShow();
    void show(TFT_eSprite *screen);
    void handleButtons();
};
