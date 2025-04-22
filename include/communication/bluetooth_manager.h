#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include "config.h"

#include <ArduinoBLE.h>
#include <ArduinoJson.h>
#include <functional>

class BluetoothManager
{
    static BluetoothManager *instance; // Singleton-like static instance

    std::function<void(const Config &, const String &)> onConfigReceived;
    BLEService configService{"180C"};
    BLECharacteristic configChar{"2A56", BLEWrite, 512};

public:
    BluetoothManager(std::function<void(const Config &, const String &)> callback)
        : onConfigReceived(callback)
    {
        instance = this;
    }

    void begin()
    {
        if (!BLE.begin())
        {
            Serial.println("BLE init failed");
            return;
        }

        BLE.setLocalName("IoTConfigDevice");
        configService.addCharacteristic(configChar);
        BLE.setAdvertisedService(configService);
        BLE.addService(configService);

        // Set static event handler
        configChar.setEventHandler(BLEWritten, BluetoothManager::onWrite);
        BLE.advertise();
    }

    void update()
    {
        BLE.poll();
    }

private:
    static void onWrite(BLEDevice central, BLECharacteristic characteristic)
    {
        if (instance)
        {
            instance->handleWrite(characteristic);
        }
    }

    void handleWrite(BLECharacteristic &chr)
    {
        String _jsonStr;
        for (int i = 0; i < chr.valueLength(); ++i)
        {
            char c = static_cast<char>(chr.value()[i]);
            if (isPrintable(c))
            {
                _jsonStr += c;
            }
        }
        parseJson(_jsonStr);
    }

    void parseJson(const String &jsonStr)
    {
        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, jsonStr);
        if (error)
        {
            Serial.println(error.c_str());
            Serial.println(jsonStr);
            Serial.println("Failed to parse JSON from Bluetooth");
            return;
        }

        Config config;
        config.device_uid = doc["device_uid"].as<String>();
        config.wifi.ssid = doc["wifi"]["ssid"].as<String>();
        config.wifi.password = doc["wifi"]["password"].as<String>();
        config.mqtt.broker = doc["mqtt"]["broker"].as<String>();
        config.mqtt.port = doc["mqtt"]["port"].as<uint16_t>();
        config.mqtt.topic = doc["mqtt"]["topic"].as<String>();

        onConfigReceived(config, static_cast<const String &>(jsonStr));
    }
};

BluetoothManager *BluetoothManager::instance = nullptr;

#endif
