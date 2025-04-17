#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <ArduinoMQTTClient.h>
#include <WiFiS3.h>
#include <communication/credentials.h>
#include <communication/wifi_manager.h>

class MQTTManager
{
public:
    MQTTManager();
    void begin(const char *device_name, uint32_t timeout_ms = 15000);
    void loop();
    bool is_connected() const;
    void publish(const String &topic, const String &message);
    void subscribe(const String &topic);
    void retry();

private:
    Credentials _credentials;
    WiFiManager _wifi_manager;
    WiFiClient _wifi_client;
    MqttClient _mqtt_client; // Changed from MQTTClient to MqttClient
    String _client_id;
    bool _mqtt_connected;
    bool _last_connect_failed; // Added to track last connection failure
    unsigned long _last_connect_attempt = 0;
    unsigned long _reconnect_interval_ms = 5000;
    static MQTTManager *_instance; // Static instance for callback

    void connect_to_mqtt();
    static void on_message_received(int message_size);
};

#endif