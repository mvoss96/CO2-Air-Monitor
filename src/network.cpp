#include <WiFi.h>
#include <WiFiManager.h>
#include <Preferences.h>
#include "network.h"

const int RECONNECT_ATTEMPT_INTERVAL = 2000;
bool wifiStarted = false;
const char *DEVICENAME = "CO2-Display";
char mqtt_server[40] = "your_mqtt_server";
int mqtt_port = 1883;
char mqtt_username[40];
char mqtt_password[40];
char mqtt_topic[40] = "your_topic";

Preferences preferences;
WiFiManager wm;
WiFiManagerParameter custom_mqtt_server("server", "MQTT Server", mqtt_server, 40);
WiFiManagerParameter custom_mqtt_port("port", "MQTT Port", String(mqtt_port).c_str(), 6);
WiFiManagerParameter custom_mqtt_username("username", "MQTT Username", mqtt_username, 40);
WiFiManagerParameter custom_mqtt_password("password", "MQTT Password", mqtt_password, 40);
WiFiManagerParameter custom_mqtt_topic("topic", "MQTT Topic", mqtt_topic, 40);

String translateWiFiStatus(wl_status_t status)
{
    switch (status)
    {
    case WL_NO_SHIELD:
        return "No shield";
    case WL_IDLE_STATUS:
        return "Idle";
    case WL_NO_SSID_AVAIL:
        return "No ssid";
    case WL_SCAN_COMPLETED:
        return "Scan Completed";
    case WL_CONNECTED:
        return "Connected";
    case WL_CONNECT_FAILED:
        return "Connect failed";
    case WL_CONNECTION_LOST:
        return "Connection lost";
    case WL_DISCONNECTED:
        return "Disconnected";
    default:
        return "N/A";
    }
}

void printMqttConfig()
{
    Serial.println("MQTT Config:");
    Serial.print("Server: ");
    Serial.println(mqtt_server);
    Serial.print("Port: ");
    Serial.println(mqtt_port);
    Serial.print("Username: ");
    Serial.println(mqtt_username);
    Serial.print("Password: ");
    Serial.println(mqtt_password);
    Serial.print("Topic: ");
    Serial.println(mqtt_topic);
}

void saveParamsCallback()
{
    strcpy(mqtt_server, custom_mqtt_server.getValue());
    mqtt_port = atoi(custom_mqtt_port.getValue());
    strcpy(mqtt_username, custom_mqtt_username.getValue());
    strcpy(mqtt_password, custom_mqtt_password.getValue());
    strcpy(mqtt_topic, custom_mqtt_topic.getValue());
    preferences.putString("mqtt_server", mqtt_server);
    preferences.putInt("mqtt_port", mqtt_port);
    preferences.putString("mqtt_username", mqtt_username);
    preferences.putString("mqtt_password", mqtt_password);
    preferences.putString("mqtt_topic", mqtt_topic);
    printMqttConfig();
}

void wifiSetup()
{
    preferences.begin("mqtt_config", false);
    strcpy(mqtt_server, preferences.getString("mqtt_server", "default_mqtt_server").c_str());
    mqtt_port = preferences.getInt("mqtt_port", 1883);
    strcpy(mqtt_username, preferences.getString("mqtt_username", "default_username").c_str());
    strcpy(mqtt_password, preferences.getString("mqtt_password", "default_password").c_str());
    strcpy(mqtt_topic, preferences.getString("mqtt_topic", "default_mqtt_topic").c_str());
    printMqttConfig();

    custom_mqtt_server.setValue(mqtt_server, 40);
    custom_mqtt_port.setValue(String(mqtt_port).c_str(), 6);
    custom_mqtt_username.setValue(mqtt_username, 40);
    custom_mqtt_password.setValue(mqtt_password, 40);
    custom_mqtt_topic.setValue(mqtt_topic, 40);

    wm.addParameter(&custom_mqtt_server);
    wm.addParameter(&custom_mqtt_port);
    wm.addParameter(&custom_mqtt_username);
    wm.addParameter(&custom_mqtt_password);
    wm.addParameter(&custom_mqtt_topic);

    // std::vector<const char *> menu = {"wifi", "info", "param", "close", "sep", "erase", "restart", "exit"};
    // wm.setMenu(menu); // custom menu, pass vector
    wm.setConnectTimeout(10);
    wm.setParamsPage(true);
    wm.setConfigPortalBlocking(false);
    wm.setSaveParamsCallback(saveParamsCallback);

    // Start WiFiManager if no WiFi credentials are saved
    if (!wm.getWiFiIsSaved())
    {
        Serial.println("Starting AP");
        wm.startConfigPortal(DEVICENAME);
    }
    else
    {
        Serial.println("Connecting to WiFi");
        wm.setEnableConfigPortal(false); // Disable config portal so that it doesn't start when connection fails
        wm.autoConnect(DEVICENAME);
    }
}

void wifiReset()
{
    Serial.println("Resetting WiFi settings");
    wm.resetSettings();
    WiFi.disconnect();
    wm.setEnableConfigPortal(true);
    wm.autoConnect(DEVICENAME);
}

void wifiLoop()
{
    static long reconnect_timer = 0;
    if (WiFi.status() == WL_CONNECTED && !wifiStarted)
    {
        wm.startWebPortal();
        wifiStarted = true;
    }
    if (WiFi.status() != WL_CONNECTED && millis() - reconnect_timer > RECONNECT_ATTEMPT_INTERVAL && wm.getWiFiIsSaved())
    {
        wm.setEnableConfigPortal(false);
        wm.autoConnect(DEVICENAME);
    }
    wm.process();
}