#pragma once
#include "display.h"

class Vertical170x320Display : public AbstractDisplay
{
private:
    TFT_eSprite screen = TFT_eSprite(&tft);
    void drawStatic();
    void drawCo2();
    void drawWifiStatus();
    void drawTemperature();
    void drawHumidity();
    

public:
    Vertical170x320Display(Sensor *s) : AbstractDisplay(s) {}
    void begin() override;
    void updateDisplay() override;
    void showErrorScreen(String message);
};