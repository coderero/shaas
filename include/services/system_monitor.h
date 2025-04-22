#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include <communication/bluetooth_manager.h>
#include <communication/wifi_manager.h>
#include <communication/mqtt_manager.h>
#include <communication/config_manager.h>
#include <services/whitelist_manager.h>
#include <services/security.h>

#include <utils/protobuf.h>

#include <pb_decode.h>
#include <pb_encode.h>
#include <transporter.pb.h>

enum class SystemState
{
    WAIT_CONFIG,
    CONNECT_WIFI,
    CONNECT_MQTT,
    READY
};

class SystemMonitor
{
private:
    BluetoothManager *bt = nullptr;
    WiFiManager *wifi = nullptr;
    MQTTManager *mqtt = nullptr;
    Security *security = nullptr;

    WhiteListManager whitelistManager;
    Config config;
    ConfigManager configManager;

    SystemState state = SystemState::WAIT_CONFIG;

    // Static pointer to the singleton instance
    static SystemMonitor *instance;

    // Static callback wrapper for MQTT
    static void mqtt_callback_wrapper(int messageSize)
    {
        Serial.println("SystemMonitor: MQTT callback invoked");
        if (instance && instance->mqtt)
        {
            MqttClient &mqttClient = instance->mqtt->getMqttClient();

            String topic = mqttClient.messageTopic();

            // Read the payload
            String msg = "";
            char c;
            for (int i = 0; i < messageSize; i++)
            {
                c = mqttClient.read();
                msg += c;
            }

            instance->mqtt_callback_manager(topic.c_str(), (uint8_t *)msg.c_str(), messageSize);
        }
        else
        {
            Serial.println("SystemMonitor: Error - No instance available in callback");
        }
    }

public:
    void init()
    {
        Serial.println("SystemMonitor: Initializing...");
        configManager.begin();
        instance = this; // Set the singleton instance

        if (configManager.load(config))
        {
            Serial.println("SystemMonitor: Config loaded successfully");
            Serial.print("SystemMonitor: MQTT Broker: ");
            Serial.println(config.mqtt.broker); // Log broker
            Serial.print("SystemMonitor: MQTT Port: ");
            Serial.println(config.mqtt.port);
            Serial.print("SystemMonitor: MQTT Topic: ");
            Serial.println(config.mqtt.topic);

            wifi = new WiFiManager(config.wifi);
            mqtt = new MQTTManager(config.mqtt);
            wifi->begin();
            mqtt->begin();                             // Use the static wrapper
            mqtt->set_callback(mqtt_callback_wrapper); // Set the callback
            whitelistManager.init(mqtt, config.device_uid);
            state = SystemState::CONNECT_WIFI;
        }
        else
        {
            Serial.println("SystemMonitor: No config found, starting Bluetooth");
            bt = new BluetoothManager([this](const Config &c, const String &json)
                                      {
                Serial.println("SystemMonitor: Bluetooth config received");
                config = c;
                configManager.save(json);

                Serial.print("SystemMonitor: MQTT Broker: ");
                Serial.println(config.mqtt.broker);
                Serial.print("SystemMonitor: MQTT Port: ");
                Serial.println(config.mqtt.port);
                Serial.print("SystemMonitor: MQTT Topic: ");
                Serial.println(config.mqtt.topic);

                wifi = new WiFiManager(config.wifi);
                mqtt = new MQTTManager(config.mqtt);
                wifi->begin();
                mqtt->begin();
                mqtt->set_callback(mqtt_callback_wrapper);      // Set the callback
                whitelistManager.init(mqtt, config.device_uid); // Initialize whitelist manager
                state = SystemState::CONNECT_WIFI; });
            bt->begin();
        }
    }

    void update()
    {
        static unsigned long last_status_check = 0;
        const unsigned long status_interval = 5000; // Check every 5 seconds

        switch (state)
        {
        case SystemState::WAIT_CONFIG:
            if (bt)
            {
                bt->update();
            }
            break;

        case SystemState::CONNECT_WIFI:
            if (wifi->update())
            {
                Serial.println("SystemMonitor: WiFi connected, moving to CONNECT_MQTT");
                state = SystemState::CONNECT_MQTT;
            }
            break;

        case SystemState::CONNECT_MQTT:
            if (!wifi->is_connected())
            {
                Serial.println("SystemMonitor: WiFi disconnected, reverting to WAIT_CONFIG");
                state = SystemState::WAIT_CONFIG;
                return;
            }

            mqtt->update();

            if (mqtt->is_connected())
            {
                Serial.println("SystemMonitor: MQTT connected, initializing whitelist and security");
                whitelistManager.init(mqtt, config.device_uid);

                // Publish a test message to verify MQTT is working
                mqtt->publish("device/arduino/test", "Test message from SystemMonitor");
                Serial.println("SystemMonitor: Published test message to device/arduino/test");

                if (!security)
                {
                    security = new Security(&whitelistManager);
                    if (!security->init(&whitelistManager))
                    {
                        Serial.println("SystemMonitor: Failed to initialize Security");
                        return;
                    }

                    Serial.println("SystemMonitor: Security module ready");
                }

                String topic = "arduino/" + config.device_uid + "/rfid";
                mqtt->subscribe(topic.c_str());
                state = SystemState::READY;
            }
            break;

        case SystemState::READY:
            if (!wifi->is_connected())
            {
                Serial.println("SystemMonitor: WiFi disconnected, reverting to WAIT_CONFIG");
                state = SystemState::WAIT_CONFIG;
                return;
            }
            if (!mqtt->is_connected())
            {
                Serial.println("SystemMonitor: MQTT disconnected, reverting to CONNECT_MQTT");
                state = SystemState::CONNECT_MQTT;
                return;
            }

            mqtt->update();
            security->handle();
            break;
        }
    }

    void mqtt_callback_manager(const char *topic, uint8_t *payload, unsigned int length)
    {
        String rfid_topic = "arduino/" + config.device_uid + "/rfid";
        char registration_request_id[128] = {0};

        if (strcmp(topic, rfid_topic.c_str()) == 0)
        {
            pb_istream_t stream = pb_istream_from_buffer(payload, length);

            transporter_RfidEnvelope rfidEnvelope = transporter_RfidEnvelope_init_zero;
            rfidEnvelope.cb_payload.funcs.decode = msg_callback;
            rfidEnvelope.cb_payload.arg = registration_request_id;

            if (pb_decode(&stream, transporter_RfidEnvelope_fields, &rfidEnvelope))
            {
                if (rfidEnvelope.which_payload == transporter_RfidEnvelope_register_request_tag)
                {
                    whitelistManager.set_registration_request_id(registration_request_id);
                    whitelistManager.set_mode_registration();
                }
            }
            else
            {
                Serial.print("SystemMonitor: Failed to decode RFID envelope: ");
                Serial.println(PB_GET_ERROR(&stream));
            }
        }
    }
    // Destructor to clean up dynamic allocations
    ~SystemMonitor()
    {
        delete bt;
        delete wifi;
        delete mqtt;
        delete security;
    }
};

// Define and initialize the static instance pointer
SystemMonitor *SystemMonitor::instance = nullptr;

#endif