// --- Config.h ---
#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

struct WiFiConfig
{
    String ssid;
    String password;
};

struct MQTTConfig
{
    String broker;
    uint16_t port;
    String topic;
    String username;
    String password;
};

struct Config
{
    String device_uid;
    WiFiConfig wifi;
    MQTTConfig mqtt;
};

#endif
