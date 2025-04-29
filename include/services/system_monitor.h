#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include <communication/bluetooth_manager.h>
#include <communication/config_manager.h>
#include <communication/wifi_manager.h>
#include <communication/mqtt_manager.h>
#include <communication/serial_module.h>
#include <services/whitelist_manager.h>
#include <services/sensor_manager.h>
#include <services/config_engine.h>
#include <services/security.h>
#include <devices/relay_control.h>

#include <modules/mux.h>

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

    Mux mux;
    Config config;
    ConfigEngine configEngine;
    SerialModule serialModule;
    RelayControl relayControl;
    ConfigManager configManager;
    SensorManager sensorManager;
    WhiteListManager whitelistManager;

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
            configureBasicConfig();
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

        if (serialModule.init(Serial1))
        {
            Serial.println("SystemMonitor: SerialModule initialized successfully");

            if (relayControl.init(&serialModule))
            {
                Serial.println("SystemMonitor: RelayControl initialized successfully");
            }
            else
            {
                Serial.println("SystemMonitor: Failed to initialize RelayControl");
            }
        }
        else
        {
            Serial.println("SystemMonitor: Failed to initialize SerialModule");
        }

        int muxSelectionPins[] = {10, 5, 8, 9};               // S0, S1, S2, S3 pins
        mux.init(3, muxSelectionPins, 4, DIGITAL, MUX_INPUT); // Signal pin on A0

        // After MQTT and ConfigEngine are initialized:
        if (state == SystemState::CONNECT_WIFI || state == SystemState::CONNECT_MQTT)
        {
            // Initialize SensorManager with mux
            if (sensorManager.init(&configEngine, mqtt, &mux, config.device_uid))
            {
                Serial.println("SystemMonitor: SensorManager initialized successfully");
            }
            else
            {
                Serial.println("SystemMonitor: Failed to initialize SensorManager");
            }
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

                uint8_t buffer[256];
                pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
                transporter_RelayStateSync relayStateSync = transporter_RelayStateSync_init_zero;

                if (!pb_encode(&stream, transporter_RelayStateSync_fields, &relayStateSync))
                {
                    Serial.print("SystemMonitor: Failed to encode RelayStateSync: ");
                    Serial.println(PB_GET_ERROR(&stream));
                    return;
                }

                String relayStateTopic = "arduino/" + config.device_uid + "/relay/full";
                mqtt->publish(relayStateTopic.c_str(), buffer, stream.bytes_written);
                mqtt->publish("device/arduino/test", "Test message from SystemMonitor");

                if (!security)
                {
                    security = new Security(&whitelistManager);
                    if (!security->init(&whitelistManager))
                    {
                        Serial.println("SystemMonitor: Failed to initialize Security");
                        return;
                    }
                }

                String wifi_config = "arduino/" + config.device_uid + "/wifi";
                String rfid_topic = "arduino/" + config.device_uid + "/rfid";
                String config_topic = "arduino/" + config.device_uid + "/config";
                String config_removal = "arduino/" + config.device_uid + "/config/remove";
                String relay_state = "arduino/" + config.device_uid + "/relay";
                String factory_reset = "arduino/" + config.device_uid + "/factory_reset";
                mqtt->subscribe(wifi_config.c_str());
                mqtt->subscribe(rfid_topic.c_str());
                mqtt->subscribe(config_topic.c_str());
                mqtt->subscribe(config_removal.c_str());
                mqtt->subscribe(relay_state.c_str());
                mqtt->subscribe(factory_reset.c_str());
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
            relayControl.handleResponses();
            sensorManager.update();
            break;
        }
    }

    void mqtt_callback_manager(const char *topic, uint8_t *payload, unsigned int length)
    {
        String wifi_config = "arduino/" + config.device_uid + "/wifi";
        String rfid_topic = "arduino/" + config.device_uid + "/rfid";
        String config_module = "arduino/" + config.device_uid + "/config";
        String config_removal = "arduino/" + config.device_uid + "/config/remove";
        String relay_state = "arduino/" + config.device_uid + "/relay";
        String factory_reset = "arduino/" + config.device_uid + "/factory_reset";
        if (strcmp(topic, wifi_config.c_str()) == 0)
        {
            handle_wifi_credentials(payload, length);
        }

        if (strcmp(topic, rfid_topic.c_str()) == 0)
        {
            handle_security(payload, length);
        }

        if (strcmp(topic, config_module.c_str()) == 0)
        {
            handle_config_manager(payload, length);
        }

        if (strcmp(topic, config_removal.c_str()) == 0)
        {
            handle_config_removal(payload, length);
        }

        if (strcmp(topic, relay_state.c_str()) == 0)
        {
            handle_relay_toggle(payload, length);
        }

        if (strcmp(topic, factory_reset.c_str()) == 0)
        {
            for (int i = 0; i < EEPROM.length(); i++)
            {
                EEPROM.write(i, 0);
            }

            NVIC_SystemReset();
        }
    }

    void handle_config_removal(uint8_t *payload, unsigned int length)
    {
        Serial.println("SystemMonitor: Received ConfigRemoval");
        pb_istream_t stream = pb_istream_from_buffer(payload, length);
        transporter_ConfigRemoval config_removal = transporter_ConfigRemoval_init_zero;
        if (pb_decode(&stream, transporter_ConfigRemoval_fields, &config_removal))
        {
            switch (config_removal.which_payload)
            {
            case transporter_ConfigRemoval_climate_tag:
                configEngine.delete_climate_config(config_removal.payload.climate.id);
                break;
            case transporter_ConfigRemoval_ldr_tag:
                configEngine.delete_ldr_config(config_removal.payload.ldr.id);
                break;
            case transporter_ConfigRemoval_motion_tag:
                configEngine.delete_motion_config(config_removal.payload.motion.id);
                break;
            default:
                break;
            }

            NVIC_SystemReset();
        }
        else
        {
            Serial.print("SystemMonitor: Failed to decode ConfigRemoval: ");
            Serial.println(PB_GET_ERROR(&stream));
        }
    }

    void handle_wifi_credentials(uint8_t *payload, unsigned int length)
    {
        Config config;
        char ssid[32], password[32];

        pb_istream_t stream = pb_istream_from_buffer(payload, length);
        transporter_WifiCredentials wifi_credentials = transporter_WifiCredentials_init_zero;
        wifi_credentials.ssid.funcs.decode = decode_string;
        wifi_credentials.password.funcs.decode = decode_string;
        wifi_credentials.ssid.arg = (void *)ssid;
        wifi_credentials.password.arg = (void *)password;

        if (pb_decode(&stream, transporter_WifiCredentials_fields, &wifi_credentials))
        {
            Serial.print("SystemMonitor: Received WiFi credentials: ");
            Serial.print(ssid);
            Serial.print(", ");
            Serial.println(password);

            config.wifi.ssid = String(ssid);
            config.wifi.password = String(password);
            configManager.update_config(config);

            NVIC_SystemReset();
        }
        else
        {
            Serial.print("SystemMonitor: Failed to decode WiFi credentials: ");
            Serial.println(PB_GET_ERROR(&stream));
        }
    }

    void handle_relay_toggle(uint8_t *payload, unsigned int length)
    {
        pb_istream_t stream = pb_istream_from_buffer(payload, length);
        transporter_RelayState relayState = transporter_RelayState_init_zero;

        if (pb_decode(&stream, transporter_RelayState_fields, &relayState))
        {
            relayControl.toggleRelay(relayState.type, relayState.port, relayState.state);
        }
        else
        {
            Serial.print("SystemMonitor: Failed to decode RelayState: ");
            Serial.println(PB_GET_ERROR(&stream));
        }
    }

    void handle_security(uint8_t *payload, unsigned int length)
    {
        CallbackSharedData shared_data = {0};
        pb_istream_t stream = pb_istream_from_buffer(payload, length);

        transporter_RfidEnvelope rfidEnvelope = transporter_RfidEnvelope_init_zero;
        rfidEnvelope.cb_payload.funcs.decode = msg_callback;
        rfidEnvelope.cb_payload.arg = &shared_data;

        if (pb_decode(&stream, transporter_RfidEnvelope_fields, &rfidEnvelope))
        {
            switch (rfidEnvelope.which_payload)
            {
            case transporter_RfidEnvelope_register_request_tag:
                whitelistManager.set_registration_request_id(shared_data.registration_id);
                whitelistManager.set_mode_registration();
                break;
            case transporter_RfidEnvelope_revoke_request_tag:
                uint8_t uid_copy[MAX_UID_LENGTH];
                memcpy(uid_copy, shared_data.uid_buffer, shared_data.uid_length);
                whitelistManager.delete_uid(uid_copy, shared_data.uid_length);
                break;
            default:
                break;
            }
        }
        else
        {
            Serial.print("SystemMonitor: Failed to decode RFID envelope: ");
            Serial.println(PB_GET_ERROR(&stream));
        }
    };

    void handle_config_manager(uint8_t *payload, unsigned int length)
    {
        Serial.println("Recieved Config");
        pb_istream_t stream = pb_istream_from_buffer(payload, length);

        transporter_ConfigTopic config = transporter_ConfigTopic_init_zero;
        if (pb_decode(&stream, transporter_ConfigTopic_fields, &config))
        {
            config_data _config;
            ldr l, ldrs[MAX_LDR];
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

                configEngine.set_climate_config(c);
                break;

            case transporter_ConfigTopic_ldr_tag:
                l.id = config.payload.ldr.id;
                l.port = config.payload.ldr.port;
                configEngine.set_ldr_config(l);
                break;

            case transporter_ConfigTopic_motion_tag:
                m.id = config.payload.motion.id;
                m.port = config.payload.motion.port;
                m.relay_type = config.payload.motion.relay_type;
                m.relay_port = config.payload.motion.relay_port;

                configEngine.set_motion_config(m);
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
                    _config.motions[i].relay_type = config.payload.full_config.motions[i].relay_type;
                    _config.motions[i].relay_port = config.payload.full_config.motions[i].relay_port;
                }

                _config.version = 1;
                _config.size = sizeof(_config);
                _config.climate_size = config.payload.full_config.climates_count;
                _config.ldr_size = config.payload.full_config.ldrs_count;
                _config.motion_size = config.payload.full_config.motions_count;

                configEngine.set_full_config(_config);
                break;
            default:
                break;
            }

            configEngine.save_config();
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