#include <WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "network.h"

bool wifiEnabled = true;
bool mqttEnabled = true;
bool wifiStarted = false;
char chipIdStr[32];
char mqtt_server[40] = "your_mqtt_server";
char mqtt_username[40];
char mqtt_password[40];
char mqtt_topic[40] = "your_topic";
int mqtt_port = 1883;

const char DEVICENAME[] = "CO2Display";
const unsigned int WIFI_RECONNECT_ATTEMPT_INTERVAL = 2000;
const unsigned int MQTT_RECONNECT_ATTEMPT_INTERVAL = 5000;
static long wifiReconnectTimer = 0; // Timer for tracking WiFi reconnection attempts
static long mqttReconnectTimer = 0; // Timer for tracking MQTT reconnection attempts

static Preferences preferences;
static WiFiManager wm;
static WiFiManagerParameter custom_mqtt_server("server", "MQTT Server", mqtt_server, 40);
static WiFiManagerParameter custom_mqtt_port("port", "MQTT Port", String(mqtt_port).c_str(), 6);
static WiFiManagerParameter custom_mqtt_username("username", "MQTT Username", mqtt_username, 40);
static WiFiManagerParameter custom_mqtt_password("password", "MQTT Password", mqtt_password, 40);
static WiFiManagerParameter custom_mqtt_topic("topic", "MQTT Topic", mqtt_topic, 40);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

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

void setWifiEnabled(bool enabled)
{
    wifiEnabled = enabled;
    preferences.putBool("wifiEnabled", enabled);
    ESP.restart(); // Restart to apply changes
}

void setMqttEnabled(bool enabled)
{
    mqttEnabled = enabled;
    preferences.putBool("mqttEnabled", enabled);
    ESP.restart(); // Restart to apply changes
}

char* getChipID()
{
    return chipIdStr;
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

void mqttHomeAssistandDiscovery()
{
    if (mqttClient.connected())
    {
        Serial.println("Publishing MQTT Home Assistant Discovery");
        String baseTopic = "homeassistant/sensor/" + String(chipIdStr);
        // CO2 Sensor
        String co2Topic = baseTopic + "/co2/config";
        JsonDocument co2Doc;
        co2Doc["name"] = "carbon_dioxide";
        co2Doc["device_class"] = "carbon_dioxide";
        co2Doc["state_topic"] = String(mqtt_topic) + "/" + String(chipIdStr) + "/co2";
        co2Doc["unit_of_measurement"] = "ppm";
        co2Doc["unique_id"] = "co2_" + String(chipIdStr);
        co2Doc["device"]["identifiers"][0] = String(chipIdStr);
        co2Doc["device"]["name"] = String(chipIdStr);
        co2Doc["device"]["model"] = DEVICENAME;
        co2Doc["device"]["manufacturer"] = "MarcusVoss";
        String co2Payload;
        serializeJson(co2Doc, co2Payload);
        // Serial.println(co2Payload);
        mqttClient.publish(co2Topic.c_str(), co2Payload.c_str());

        // Temperature Sensor
        String temperatureTopic = baseTopic + "/temperature/config";
        JsonDocument temperatureDoc;
        temperatureDoc["name"] = "temperature";
        temperatureDoc["device_class"] = "temperature";
        temperatureDoc["state_topic"] = String(mqtt_topic) + "/" + String(chipIdStr) + "/temperature";
        temperatureDoc["unit_of_measurement"] = "°C";
        temperatureDoc["unique_id"] = "temperature_" + String(chipIdStr);
        temperatureDoc["device"]["identifiers"][0] = String(chipIdStr);
        temperatureDoc["device"]["name"] = String(chipIdStr);
        temperatureDoc["device"]["model"] = DEVICENAME;
        temperatureDoc["device"]["manufacturer"] = "MarcusVoss";
        String temperaturePayload;
        serializeJson(temperatureDoc, temperaturePayload);
        mqttClient.publish(temperatureTopic.c_str(), temperaturePayload.c_str());

        // Humidity Sensor
        String humidityTopic = baseTopic + "/humidity/config";
        JsonDocument humidityDoc;
        humidityDoc["name"] = "humidity";
        humidityDoc["device_class"] = "humidity";
        humidityDoc["state_topic"] = String(mqtt_topic) + "/" + String(chipIdStr) + "/humidity";
        humidityDoc["unit_of_measurement"] = "%";
        humidityDoc["unique_id"] = "humidity_" + String(chipIdStr);
        humidityDoc["device"]["identifiers"][0] = String(chipIdStr);
        humidityDoc["device"]["name"] = String(chipIdStr);
        humidityDoc["device"]["model"] = DEVICENAME;
        humidityDoc["device"]["manufacturer"] = "MarcusVoss";
        String humidityPayload;
        serializeJson(humidityDoc, humidityPayload);
        mqttClient.publish(humidityTopic.c_str(), humidityPayload.c_str());
    }
}

void mqttConnect()
{
    Serial.println("Attempting MQTT connection...");
    if (mqttClient.connect(chipIdStr, mqtt_username, mqtt_password))
    {
        mqttHomeAssistandDiscovery();
        Serial.println("connected");
    }
    else
    {
        Serial.print("failed, rc=");
        Serial.println(mqttClient.state());
    }
}

void mqttPublish(Sensor *sensor)
{
    String baseTopic = String(mqtt_topic) + "/" + chipIdStr;
    if (sensor->hasError())
    {
        mqttClient.publish((baseTopic + "/error").c_str(), "true");
        return;
    }
    if (sensor->isReady() && mqttClient.connected() && !sensor->hasError() && sensor->update())
    {
        mqttClient.publish((baseTopic + "/error").c_str(), "false");
        mqttClient.publish((baseTopic + "/co2").c_str(), String(sensor->getCo2Value()).c_str());
        mqttClient.publish((baseTopic + "/temperature").c_str(), String(sensor->getTemperature() / 100.0, 1).c_str());
        mqttClient.publish((baseTopic + "/humidity").c_str(), String(sensor->getHumidity() / 100.0, 1).c_str());
    }
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

void initChipID()
{
    // Generate chip ID
    String macAddress = WiFi.macAddress();
    Serial.printf("ESP32 Chip ID = %s\n", macAddress.c_str());                 // Print MAC address
    macAddress.replace(":", "");                                               // Remove colons from the MAC address
    char modifiedMac[13];                                                      // MAC address without colons is 12 characters + 1 for null terminator
    macAddress.substring(6, 17).toCharArray(modifiedMac, sizeof(modifiedMac)); // Copy the last 6 characters of the MAC address
    sprintf(chipIdStr, "%s-%s", DEVICENAME, modifiedMac);                      // Prepend the device name to the MAC address
    Serial.println(chipIdStr);
}

void wifiSetup()
{
    initChipID();
    
    preferences.begin("mqtt_config", false);
    wifiEnabled = preferences.getBool("wifiEnabled", true);
    mqttEnabled = preferences.getBool("mqttEnabled", true);
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

    if (wifiEnabled && mqttEnabled && strlen(mqtt_server) > 0 && strlen(mqtt_topic) > 0)
    {
        mqttClient.setBufferSize(512);
        mqttClient.setServer(mqtt_server, mqtt_port);
    }

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
    wm.setEnableConfigPortal(false);

    if (wifiEnabled)
    {
        wm.autoConnect();
        // Start WiFiManager if no WiFi credentials are saved
        if (wm.getWiFiIsSaved())
        {
            Serial.println("Connecting to WiFi");
            wm.setEnableConfigPortal(false); // Disable config portal so that it doesn't start when connection fails
            wm.autoConnect();
        }
        else
        {
            Serial.println("Starting AP");
            wm.setEnableConfigPortal(true);
            wm.startConfigPortal(DEVICENAME);
        }
    }
}

void wifiReset()
{
    if (!wifiEnabled)
    {
        return; // WiFi is disabled so skip WiFiReset
    }
    Serial.println("Resetting WiFi settings");
    wm.resetSettings();
    WiFi.disconnect();
    ESP.restart();
}

void handleMQTTConnection(Sensor *sensor)
{
    // Skip MQTT handling if necessary conditions are not met
    if (!wm.getWiFiIsSaved() || !mqttEnabled || strlen(mqtt_server) == 0 || strlen(mqtt_topic) == 0)
    {
        return;
    }

    // Attempt MQTT reconnection if disconnected and interval has passed
    if (!mqttClient.connected() && millis() - mqttReconnectTimer > MQTT_RECONNECT_ATTEMPT_INTERVAL)
    {
        mqttConnect();                 // Establish MQTT connection
        mqttReconnectTimer = millis(); // Reset the timer after a connection attempt
    }
    mqttPublish(sensor); // Publish sensor data to MQTT
    mqttClient.loop();   // Allow MQTT client to process incoming and outgoing messages
}

void handleWiFiConnection()
{
    // Check and handle WiFi connection status
    if (WiFi.status() == WL_CONNECTED && !wifiStarted)
    {
        wm.startWebPortal(); // Start the WiFi portal if WiFi is connected and not yet started
        wifiStarted = true;  // Mark the WiFi as started
    }
    else if (WiFi.status() == WL_DISCONNECTED && millis() - wifiReconnectTimer > WIFI_RECONNECT_ATTEMPT_INTERVAL && wm.getWiFiIsSaved())
    {
        wm.setEnableConfigPortal(false); // Disable the configuration portal
        wm.autoConnect(DEVICENAME);      // Attempt to automatically connect to WiFi
        wifiReconnectTimer = millis();   // Reset the timer after a connection attempt
    }
}

void wifiLoop(Sensor *sensor)
{
    if (!wifiEnabled)
    {
        return; // Skip the loop if WiFi is disabled
    }

    handleMQTTConnection(sensor); // Handle MQTT connection and publishing
    handleWiFiConnection();       // Handle WiFi connectivity and reconnection
    wm.process();                 // Process WiFiManager tasks
}