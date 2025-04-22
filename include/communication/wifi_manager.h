// --- WiFiManager.h ---
#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H
#include "config.h"

#include <WiFi.h>

class WiFiManager
{
    WiFiConfig config;
    int retryCount = 0;
    const int maxRetries = 3;

public:
    WiFiManager(const WiFiConfig &cfg) : config(cfg) {}

    void begin()
    {
        WiFi.begin(config.ssid.c_str(), config.password.c_str());
    }

    bool update()
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            retryCount++;
            if (retryCount <= maxRetries)
            {
                delay(1000);
                Serial.println("Connecting to WiFi...");
                Serial.print("Attempt ");
                Serial.print(retryCount);
                Serial.print(" of ");
                Serial.println(maxRetries);
                WiFi.begin(config.ssid.c_str(), config.password.c_str());
                return false;
            }
            return false;
        }
        else
        {
            Serial.println("Connected to WiFi");
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());
            Serial.print("Signal Strength: ");
            Serial.println(WiFi.RSSI());
            Serial.print("SSID: ");
            Serial.println(WiFi.SSID());
            Serial.print("Gateway: ");
            Serial.println(WiFi.gatewayIP());
            Serial.print("Subnet Mask: ");
            Serial.println(WiFi.subnetMask());
            Serial.print("DNS: ");
            Serial.println(WiFi.dnsIP());

            retryCount = 0; // Reset retry count on successful connection
            return true;
        }
    }

    bool is_connected()
    {
        return WiFi.status() == WL_CONNECTED;
    }
};

#endif