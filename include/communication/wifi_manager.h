#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFiS3.h>
#include <EEPROM.h>
#include <communication/credentials.h>

class WiFiManager
{
private:
    void connect_to_wifi();
    void raw_connect_to_wifi();
    void fallback_to_ble();

    Credentials &_credentials;
    network_config _cfg;
    bool _connected, _is_configured;
    uint32_t _timeout_ms;

public:
    WiFiManager(Credentials &creds);
    void begin(const char *device_name, uint32_t timeout_ms = 15000);
    void loop(); // To poll credentials BLE engine regularly
    void retry();
    bool is_connected() const;
    const network_config &get_config() const;
};

#endif // WIFI_MANAGER_H
