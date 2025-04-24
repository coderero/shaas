#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include <communication/bluetooth_manager.h>
#include <communication/config_manager.h>
#include <communication/wifi_manager.h>
#include <communication/mqtt_manager.h>
#include <services/whitelist_manager.h>
#include <services/config_engine.h>
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
    ConfigEngine configEngine;

    SystemState state = SystemState::WAIT_CONFIG;

    // Static pointer to the singleton instance
    static SystemMonitor *instance;

    // Static callback wrapper for MQTT
    static void mqtt_callback_wrapper(int messageSize)
    {
        if (instance && instance->mqtt)
        {
            MqttClient &mqttClient = instance->mqtt->getMqttClient();

            String topic = mqttClient.messageTopic();

            String msg = "";
            char c;
            for (int i = 0; i < messageSize; i++)
            {
                c = mqttClient.read();
                msg += c;
            }

            instance->mqtt_callback_manager(topic.c_str(), (uint8_t *)msg.c_str(), messageSize);
        }
    }

    void configureBasicConfig()
    {
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
        configEngine.init();
        whitelistManager.init(mqtt, config.device_uid);
        state = SystemState::CONNECT_WIFI;
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
            configEngine.init();
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
                                          configureBasicConfig(); });
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
                }

                String rfid_topic = "arduino/" + config.device_uid + "/rfid";
                String config_topic = "arduino/" + config.device_uid + "/config";
                mqtt->subscribe(rfid_topic.c_str());
                mqtt->subscribe(config_topic.c_str());
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
            whitelistManager.update();
            security->handle();
            break;
        }
    }

    void mqtt_callback_manager(const char *topic, uint8_t *payload, unsigned int length)
    {
        String rfid_topic = "arduino/" + config.device_uid + "/rfid";
        String config_module = "arduino/" + config.device_uid + "/config";
        if (strcmp(topic, rfid_topic.c_str()) == 0)
        {
            handle_security(topic, payload, length);
        }

        if (strcmp(topic, config_module.c_str()) == 0)
        {
            pb_istream_t stream = pb_istream_from_buffer(payload, length);

            transporter_ConfigTopic config = transporter_ConfigTopic_init_zero;
            if (pb_decode(&stream, transporter_ConfigTopic_fields, &config))
            {
                config_data _config;
                ldr l, ldrs[MAX_LDR];
                relay r, relays[MAX_RELAY];
                motion m, motions[MAX_MOTION];
                climate c, climates[MAX_CLIMATE];

                switch (config.which_payload)
                {
                case transporter_ConfigTopic_climate_tag:
                    c.id = config.payload.climate.id;
                    c.dht22_port = config.payload.climate.dht22_port;
                    c.aqi_port = config.payload.climate.aqi_port;
                    c.has_buzzer = config.payload.climate.has_buzzers;
                    c.buzzer_port = config.payload.climate.buzzer_port;

                    Serial.println("SystemMonitor: Climate config received");
                    Serial.print("SystemMonitor: Climate ID: ");
                    Serial.println(c.id);
                    Serial.print("SystemMonitor: DHT22 Port: ");
                    Serial.println(c.dht22_port);
                    Serial.print("SystemMonitor: AQI Port: ");
                    Serial.println(c.aqi_port);
                    Serial.print("SystemMonitor: Has Buzzer: ");
                    Serial.println(c.has_buzzer);
                    Serial.print("SystemMonitor: Buzzer Port: ");
                    Serial.println(c.buzzer_port);

                    break;

                case transporter_ConfigTopic_ldr_tag:
                    l.id = config.payload.ldr.id;
                    l.port = config.payload.ldr.port;

                    Serial.println("SystemMonitor: LDR config received");
                    Serial.print("SystemMonitor: LDR ID: ");
                    Serial.println(l.id);
                    Serial.print("SystemMonitor: LDR Port: ");
                    Serial.println(l.port);
                    break;

                case transporter_ConfigTopic_motion_tag:
                    m.id = config.payload.motion.id;
                    m.port = config.payload.motion.port;
                    m.relay_channel = config.payload.motion.relay_channel;
                    m.relay_id = config.payload.motion.relay_id;

                    Serial.println("SystemMonitor: Motion config received");
                    Serial.print("SystemMonitor: Motion ID: ");
                    Serial.println(m.id);
                    Serial.print("SystemMonitor: Motion Port: ");
                    Serial.println(m.port);
                    Serial.print("SystemMonitor: Relay Channel: ");
                    Serial.println(m.relay_channel);
                    Serial.print("SystemMonitor: Relay ID: ");
                    Serial.println(m.relay_id);
                    break;
                case transporter_ConfigTopic_relay_tag:
                    r.id = config.payload.relay.id;
                    r.type = config.payload.relay.type;

                    Serial.println("SystemMonitor: Relay config received");
                    Serial.print("SystemMonitor: Relay ID: ");
                    Serial.println(r.id);
                    Serial.print("SystemMonitor: Relay Type: ");
                    Serial.println(r.type);
                    break;

                case transporter_ConfigTopic_full_config_tag:

                    for (int i = 0; i < config.payload.full_config.climates_count; i++)
                    {
                        _config.climates[i].id = config.payload.full_config.climates[i].id;
                        _config.climates[i].dht22_port = config.payload.full_config.climates[i].dht22_port;
                        _config.climates[i].aqi_port = config.payload.full_config.climates[i].aqi_port;
                        _config.climates[i].has_buzzer = config.payload.full_config.climates[i].has_buzzers;
                        _config.climates[i].buzzer_port = config.payload.full_config.climates[i].buzzer_port;
                    }

                    for (int i = 0; i < config.payload.full_config.ldrs_count; i++)
                    {
                        _config.ldrs[i].id = config.payload.full_config.ldrs[i].id;
                        _config.ldrs[i].port = config.payload.full_config.ldrs[i].port;
                    }

                    for (int i = 0; i < config.payload.full_config.motions_count; i++)
                    {
                        _config.motions[i].id = config.payload.full_config.motions[i].id;
                        _config.motions[i].port = config.payload.full_config.motions[i].port;
                        _config.motions[i].relay_channel = config.payload.full_config.motions[i].relay_channel;
                        _config.motions[i].relay_id = config.payload.full_config.motions[i].relay_id;
                    }

                    for (int i = 0; i < config.payload.full_config.relays_count; i++)
                    {
                        _config.relays[i].id = config.payload.full_config.relays[i].id;
                        _config.relays[i].type = config.payload.full_config.relays[i].type;
                    }

                    _config.version = 1;
                    _config.size = sizeof(_config);
                    _config.climate_size = config.payload.full_config.climates_count;
                    _config.ldr_size = config.payload.full_config.ldrs_count;
                    _config.motion_size = config.payload.full_config.motions_count;
                    _config.relay_size = config.payload.full_config.relays_count;

                    Serial.println("SystemMonitor: Full config received");
                    Serial.print("SystemMonitor: Climate Size: ");
                    Serial.println(_config.climate_size);
                    Serial.print("SystemMonitor: LDR Size: ");
                    Serial.println(_config.ldr_size);
                    Serial.print("SystemMonitor: Motion Size: ");
                    Serial.println(_config.motion_size);
                    Serial.print("SystemMonitor: Relay Size: ");
                    Serial.println(_config.relay_size);
                    break;
                default:
                    break;
                }
            }
        }
    }

    void handle_security(const char *topic, uint8_t *payload, unsigned int length)
    {
        char registration_request_id[128] = {0};
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
    };

    void handle_config_manager(const char *topic, uint8_t *payload, unsigned int length)
    {
        // Handle the configuration manager callback
        String config_topic = "arduino/" + config.device_uid + "/config";
        if (strcmp(topic, config_topic.c_str()) == 0)
        {
            Serial.println("SystemMonitor: Config manager callback invoked");
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