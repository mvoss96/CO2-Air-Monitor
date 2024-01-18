#pragma once
#include "display.h"

enum menuItems : uint8_t
{
    MENU_WIFI_ENABLED,
    MENU_MQTT_ENABLED,
    MENU_ABC_ENABLED,
    MENU_TEMP_OFFSET,
    MENU_HUMID_OFFSET,
    MENU_FRC_VALUE,
    MENU_FRC_START,
    MENU_WIFI_RESET,
    MENU_REBOOT,
    MENU_EXIT,
    MENU_END,
};

class Vertical170x320Display : public AbstractDisplay
{
private:
    TFT_eSprite screen = TFT_eSprite(&tft);
    bool menuActive = false;
    bool wasClicked = false;
    bool menuClickWasHandled = false;
    uint8_t nightModeThreshold = 32;
    bool nightMode = false;
    bool inMenuItem = false;
    uint8_t menuIndex = MENU_EXIT;
    uint8_t brightness = 255;
    void drawStatic();
    void drawCo2();
    void drawWifiStatus();
    void drawTemperature();
    void drawHumidity();
    void showErrorScreen(String message);
    void showMenuScreen();
    void handleMenu();
    void drawMenuItem(const String &itemName, bool condition, const String &itemValue, int &currentHeight, const uint8_t itemHeight, const uint8_t margin);

public:
    Vertical170x320Display(Sensor *s) : AbstractDisplay(s)
    {
    }
    void begin() override;
    void updateDisplay() override;
    void setBrightness(uint8_t brightness) override;
};