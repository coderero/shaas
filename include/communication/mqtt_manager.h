// --- MQTTManager.h ---
#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H
#include "config.h"
#include <ArduinoMqttClient.h>
#include <WiFi.h>

class MQTTManager
{
private:
    MQTTConfig config;
    WiFiClient wifiClient;
    MqttClient mqttClient;

public:
    MQTTManager(const MQTTConfig &cfg) : config(cfg), mqttClient(wifiClient) {}

    void begin()
    {
        mqttClient.setId("arduinoClient");
        mqttClient.setUsernamePassword(config.username.c_str(), config.password.c_str());
        mqttClient.setKeepAliveInterval(60000);
    }

    bool update()
    {
        if (!mqttClient.connected())
        {
            return mqttClient.connect(config.broker.c_str(), config.port);
        }
        mqttClient.poll();
        return true;
    }

    bool is_connected()
    {
        return mqttClient.connected();
    }

    void publish(const char *topic, const char *payload)
    {
        mqttClient.beginMessage(topic);
        mqttClient.print(payload);
        mqttClient.endMessage();
    }

    void publish(const char *topic, const uint8_t *payload, size_t length)
    {
        mqttClient.beginMessage(topic);
        mqttClient.write(payload, length);
        mqttClient.endMessage();
    }

    void subscribe(const char *topic)
    {
        mqttClient.subscribe(topic);
    }

    void set_callback(void (*callback)(int messageSize))
    {
        Serial.println("MQTTManager: Setting callback");
        mqttClient.onMessage(callback);
    }

    MqttClient &getMqttClient()
    {
        return mqttClient;
    }
};
#endif