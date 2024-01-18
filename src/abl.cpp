#include "abl.h"

static const uint8_t NUM_SAMPLES = 100;
static const unsigned long SAMPLE_INTERVAL = 10;

unsigned int getAblValue()
{
  return analogRead(PIN_ABL);
}

uint8_t getAblBrightness() {
    static bool initialized = false;
    static uint8_t brightness = 127;
    static unsigned int samples[NUM_SAMPLES];
    static int sampleIndex = 0;
    static unsigned long lastSampleTime = 0;

    // Initialize samples array to current ABL value only once
    if (!initialized) {
        for (int i = 0; i < NUM_SAMPLES; i++) {
            samples[i] = getAblValue();
        }
        initialized = true;
    }

    unsigned int ablValue = getAblValue();
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - lastSampleTime;

    if (elapsedTime >= SAMPLE_INTERVAL) {
        samples[sampleIndex++] = ablValue;
        if (sampleIndex >= NUM_SAMPLES) {
            sampleIndex = 0;
        }


        unsigned long sum = 0;
        for (int i = 0; i < NUM_SAMPLES; i++) {
            sum += samples[i];
        }
        brightness = map(sum / NUM_SAMPLES, 0, 4095, 1, 255);
    }

    return brightness;
}
