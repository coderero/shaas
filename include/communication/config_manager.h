// --- ConfigManager.h ---
#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include "config.h"

#include <EEPROM.h>

#define EEPROM_SIZE 512
#define EEPROM_ADDR 0
#define CONFIG_MAGIC 0xABCD1234

struct StoredConfig
{
    uint32_t magic;
    Config config;
};

class ConfigManager
{
public:
    void begin()
    {
        EEPROM.begin();
    }

    bool load(Config &config)
    {
        uint32_t magic;
        String json;

        EEPROM.get(EEPROM_ADDR, magic);
        if (magic != CONFIG_MAGIC)
        {
            Serial.println("Invalid config magic");
            return false;
        }

        char buffer[EEPROM_SIZE];
        for (int i = 0; i < EEPROM_SIZE; i++)
        {
            char c = EEPROM.read(EEPROM_ADDR + sizeof(CONFIG_MAGIC) + i);
            if (c == '\0')
                break;
            buffer[i] = c;
        }

        buffer[EEPROM_SIZE - 1] = '\0';
        json = String(buffer);

        StaticJsonDocument<EEPROM_SIZE> doc;
        DeserializationError error = deserializeJson(doc, json);
        if (error)
        {
            Serial.print(F("Failed to parse JSON: "));
            Serial.println(error.c_str());
            return false;
        }

        config.device_uid = doc["device_uid"].as<String>();
        config.wifi.ssid = doc["wifi"]["ssid"].as<String>();
        config.wifi.password = doc["wifi"]["password"].as<String>();
        config.mqtt.broker = doc["mqtt"]["broker"].as<String>();
        config.mqtt.port = doc["mqtt"]["port"].as<uint16_t>();
        config.mqtt.topic = doc["mqtt"]["topic"].as<String>();
        config.mqtt.username = doc["mqtt"]["username"].as<String>();
        config.mqtt.password = doc["mqtt"]["password"].as<String>();

        return true;
    }

    bool save(const String &json)
    {
        // Save the config string to EEPROM
        EEPROM.put(EEPROM_ADDR, CONFIG_MAGIC);

        int len = min(json.length(), EEPROM_SIZE - sizeof(CONFIG_MAGIC));
        for (int i = 0; i < len; i++)
        {
            EEPROM.write(EEPROM_ADDR + sizeof(CONFIG_MAGIC) + i, json[i]);
        }

        EEPROM.write(EEPROM_ADDR + sizeof(CONFIG_MAGIC) + len, '\0'); // Null-terminate the string
        return true;
    }

    void clear()
    {
        for (int i = 0; i < EEPROM_SIZE; ++i)
        {
            EEPROM.write(i, 0);
        }
    }
};

#endif
