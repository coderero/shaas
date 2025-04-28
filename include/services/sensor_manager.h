#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <services/config_engine.h>
#include <communication/mqtt_manager.h>
#include <sensors/climate.h>
#include <sensors/ldr.h>
#include <sensors/pir.h>
#include <modules/mux.h>
#include <utils/protobuf.h>
#include <pb_encode.h>
#include <transporter.pb.h>

/**
 * @class SensorManager
 * @brief Manages sensor operations based on configuration from ConfigEngine
 *
 * This class handles:
 * - Reading from configured sensors (Climate, LDR, Motion)
 * - Processing sensor data
 * - Publishing data to appropriate MQTT topics
 */
class SensorManager
{
private:
    ConfigEngine *configEngine;
    MQTTManager *mqtt;
    String deviceId;
    Mux *mux;

    // Sensor module arrays
    Climate *climateModules[MAX_CLIMATE] = {nullptr};
    LDR *ldrModules[MAX_LDR] = {nullptr};
    PIR *pirModules[MAX_MOTION] = {nullptr};

    // Sensor reading timestamps
    unsigned long lastClimateReadTime = 0;
    unsigned long lastLdrReadTime = 0;
    unsigned long lastMotionReadTime = 0;

    // Constants
    const unsigned long SENSOR_READ_INTERVAL = 5000; // 5 seconds
    const unsigned long SENSOR_READ_INTERVAL_MOTION = 100;

    /**
     * @brief Read climate sensors and publish data
     */
    void processClimateSensors();

    /**
     * @brief Read LDR sensors and publish data
     */
    void processLdrSensors();

    /**
     * @brief Check motion sensors and publish data if motion detected
     */
    void processMotionSensors();

    /**
     * @brief Publish climate data to MQTT topic
     * @param id Climate sensor ID
     * @param temperature Temperature reading
     * @param humidity Humidity reading
     * @param aqi Air Quality Index reading
     */
    void publishClimateData(uint8_t id, float temperature, float humidity, uint32_t aqi);

    /**
     * @brief Publish LDR data to MQTT topic
     * @param id LDR sensor ID
     * @param value Light reading value
     */
    void publishLdrData(uint8_t id, uint32_t value);

    /**
     * @brief Publish relay state to MQTT topic
     * @param type Relay type
     * @param port Relay port
     * @param state Relay state
     */
    void publishRelayState(uint8_t type, uint8_t port, uint8_t state);

    /**
     * @brief Set buzzer state through mux
     * @param port Mux channel for buzzer
     * @param state Buzzer state (HIGH/LOW)
     */
    void setBuzzerState(uint8_t port, uint8_t state);

public:
    /**
     * @brief Constructor
     */
    SensorManager();

    /**
     * @brief Destructor
     */
    ~SensorManager();

    /**
     * @brief Initialize the sensor manager
     * @param configEngine Pointer to ConfigEngine instance
     * @param mqtt Pointer to MQTTManager instance
     * @param mux Pointer to Mux instance
     * @param deviceId Device ID string
     * @return True if initialization successful
     */
    bool init(ConfigEngine *configEngine, MQTTManager *mqtt, Mux *mux, const String &deviceId);

    /**
     * @brief Update method to be called in the main loop
     */
    void update();
};

#endif // SENSOR_MANAGER_H