#pragma once
#include "display.h"
#include "menu.h"

enum displayStyles : uint8_t{
    STYLE_DEFAULT,
    STYLE_NO_UPPER_COLOR,
    STYLE_END,
};

class Vertical170x320Display : public AbstractDisplay
{
private:
    TFT_eSprite screen = TFT_eSprite(&tft);
    Menu menu = Menu(this);
    const uint8_t nightModeThreshold = 32;
    bool nightMode = false;
    void drawStatic();
    void drawCo2();
    void drawWifiStatus();
    void drawTemperature();
    void drawHumidity();
    void showErrorScreen(String message);
    uint16_t determineUpperColor(int co2Value);

public:
    Vertical170x320Display(Sensor *s) : AbstractDisplay(s)
    {
    }
    void begin() override;
    void updateDisplay() override;
    void setBrightness(uint8_t brightness) override;
    void setAbcEnabled(bool enabled) override;
    void setStyle(uint8_t style) override;
};