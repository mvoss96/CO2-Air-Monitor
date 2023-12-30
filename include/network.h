#pragma once
#include <WiFi.h>

extern bool networkActive;

String translateWiFiStatus(wl_status_t status);
void wifiSetup();
void wifiLoop();