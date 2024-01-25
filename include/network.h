#pragma once
#include <WiFi.h>
#include "sensor.h"

extern bool wifiEnabled;
extern bool mqttEnabled;

extern char mqtt_server[];
extern int mqtt_port;
extern char mqtt_username[];
extern char mqtt_password[];
extern char mqtt_topic[];

String translateWiFiStatus(wl_status_t status);
void wifiSetup();
void wifiReset();
void wifiLoop(Sensor* sensor);
void setWifiEnabled(bool enabled);
void setMqttEnabled(bool enabled);
char* getChipID();
