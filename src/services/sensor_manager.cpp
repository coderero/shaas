#include <services/sensor_manager.h>

/**
 * @brief Constructor
 */
SensorManager::SensorManager() : configEngine(nullptr), mqtt(nullptr), mux(nullptr)
{
}

/**
 * @brief Destructor - clean up created sensor modules
 */
SensorManager::~SensorManager()
{
    // Clean up any climate modules
    for (int i = 0; i < MAX_CLIMATE; i++)
    {
        if (climateModules[i])
        {
            delete climateModules[i];
            climateModules[i] = nullptr;
        }
    }

    // Clean up any LDR modules
    for (int i = 0; i < MAX_LDR; i++)
    {
        if (ldrModules[i])
        {
            delete ldrModules[i];
            ldrModules[i] = nullptr;
        }
    }

    // Clean up any PIR modules
    for (int i = 0; i < MAX_MOTION; i++)
    {
        if (pirModules[i])
        {
            delete pirModules[i];
            pirModules[i] = nullptr;
        }
    }
}

/**
 * @brief Initialize the sensor manager
 */
bool SensorManager::init(ConfigEngine *configEngine, MQTTManager *mqtt, Mux *mux, const String &deviceId)
{
    if (!configEngine || !mqtt || !mux)
    {
        Serial.println("SensorManager: Invalid parameters for initialization");
        return false;
    }

    this->configEngine = configEngine;
    this->mqtt = mqtt;
    this->deviceId = deviceId;
    this->mux = mux;

    // Get the configuration data
    config_data *config = configEngine->get_configs();
    if (!config)
    {
        Serial.println("SensorManager: Failed to get configuration data");
        return false;
    }

    // Initialize Climate modules
    for (int i = 0; i < config->climate_size; i++)
    {
        climate c = config->climates[i];

        climateModules[i] = new Climate();

        // Initialize with signal pin from mux and the mux instance
        if (!climateModules[i]->init(c.id, c.dht22_port, c.aqi_port, mux->getSignalPin(), mux))
        {
            Serial.print("SensorManager: Failed to initialize Climate module ID ");
            Serial.println(c.id);
            delete climateModules[i];
            climateModules[i] = nullptr;
            continue;
        }

        Serial.print("SensorManager: Initialized Climate module ID ");
        Serial.print(c.id);
        Serial.print(" with DHT22 port ");
        Serial.print(c.dht22_port);
        Serial.print(" and AQI port ");
        Serial.println(c.aqi_port);
    }

    // Initialize LDR modules
    for (int i = 0; i < config->ldr_size; i++)
    {
        ldr l = config->ldrs[i];

        // Create LDR module instance with mux
        ldrModules[i] = new LDR();
        ldrModules[i]->init(l.id, l.port);

        Serial.print("SensorManager: Initialized LDR module ID ");
        Serial.print(l.id);
        Serial.print(" on mux port ");
        Serial.println(l.port);
    }

    // Initialize motion sensor PIR modules
    for (int i = 0; i < config->motion_size; i++)
    {
        motion m = config->motions[i];

        // Create PIR module instance with mux
        pirModules[i] = new PIR();
        pirModules[i]->init(m.id, mux);
        pirModules[i]->set_port(m.port);

        Serial.print("SensorManager: Initialized PIR motion sensor ID ");
        Serial.print(m.id);
        Serial.print(" on mux port ");
        Serial.println(m.port);
    }

    Serial.println("SensorManager: Initialization complete");
    return true;
}

/**
 * @brief Set buzzer state through mux
 */
void SensorManager::setBuzzerState(uint8_t port, uint8_t state)
{
    if (!mux)
        return;

    // Select the buzzer channel on the mux
    mux->m_mode(DIGITAL);
    mux->s_mode(MUX_OUTPUT);
    mux->channel(port);
    mux->write(state);
}

/**
 * @brief Update method to be called in the main loop
 */
void SensorManager::update()
{
    unsigned long currentTime = millis();

    // Process climate sensors at regular intervals
    if (currentTime - lastClimateReadTime >= SENSOR_READ_INTERVAL)
    {
        processClimateSensors();
        lastClimateReadTime = currentTime;
    }

    // Process LDR sensors at regular intervals
    if (currentTime - lastLdrReadTime >= SENSOR_READ_INTERVAL)
    {
        processLdrSensors();
        lastLdrReadTime = currentTime;
    }

    if (currentTime - lastMotionReadTime >= SENSOR_READ_INTERVAL_MOTION)
    {
        processMotionSensors();
        lastMotionReadTime = currentTime;
    }
}

/**
 * @brief Read climate sensors and publish data
 */
void SensorManager::processClimateSensors()
{
    if (!configEngine || !mqtt || !mux)
        return;

    config_data *config = configEngine->get_configs();
    if (!config)
        return;

    for (int i = 0; i < config->climate_size; i++)
    {
        climate c = config->climates[i];
        if (!climateModules[i])
            continue;

        // Read climate data
        climateModules[i]->read_climate_data();

        // Get sensor values
        float temperature = climateModules[i]->get_temperature();
        float humidity = climateModules[i]->get_humidity();
        uint32_t aqi = climateModules[i]->get_air_quality_index();

        // Publish climate data
        publishClimateData(c.id, temperature, humidity, aqi);

        // Check for high temperature/humidity and trigger buzzer if configured
        if (c.has_buzzer)
        {
            bool alarmCondition = (temperature > 35.0f) || (humidity > 80.0f) || (aqi > 300);
            setBuzzerState(c.buzzer_port, alarmCondition ? HIGH : LOW);
        }
    }
}

/**
 * @brief Read LDR sensors and publish data
 */
void SensorManager::processLdrSensors()
{
    if (!configEngine || !mqtt || !mux)
        return;

    config_data *config = configEngine->get_configs();
    if (!config)
        return;

    for (int i = 0; i < config->ldr_size; i++)
    {
        ldr l = config->ldrs[i];
        if (!ldrModules[i])
            continue;

        // Read LDR value
        uint32_t ldrValue = ldrModules[i]->read();

        // Publish LDR data
        publishLdrData(l.id, ldrValue);
    }
}

/**
 * @brief Check motion sensors and publish relay state if motion detected
 */
void SensorManager::processMotionSensors()
{
    if (!configEngine || !mqtt || !mux)
        return;

    config_data *config = configEngine->get_configs();
    if (!config)
        return;

    for (int i = 0; i < config->motion_size; i++)
    {
        motion m = config->motions[i];
        if (!pirModules[i])
            continue;

        // Check for motion detection
        bool motionDetected = pirModules[i]->get_movement();
        if (motionDetected)
        {
            // Publish relay state
            publishRelayState(m.relay_type, m.relay_port, HIGH);
        }
        else
        {
            // Publish relay state
            publishRelayState(m.relay_type, m.relay_port, LOW);
        }
    }
}

/**
 * @brief Publish climate data to MQTT topic
 */
void SensorManager::publishClimateData(uint8_t id, float temperature, float humidity, uint32_t aqi)
{
    if (!mqtt)
        return;

    // Create ClimateData message
    transporter_ClimateData climateData = transporter_ClimateData_init_zero;
    climateData.id = id;
    climateData.temperature = temperature;
    climateData.humidity = humidity;
    climateData.aqi = aqi;

    // Serialize the message
    uint8_t buffer[128];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    bool status = pb_encode(&stream, transporter_ClimateData_fields, &climateData);
    if (!status)
    {
        Serial.print("SensorManager: Failed to encode climate data: ");
        Serial.println(PB_GET_ERROR(&stream));
        return;
    }

    // Construct the MQTT topic
    String topic = "arduino/" + deviceId + "/climate";

    // Publish the message
    mqtt->publish(topic.c_str(), buffer, stream.bytes_written);
}

/**
 * @brief Publish LDR data to MQTT topic
 */
void SensorManager::publishLdrData(uint8_t id, uint32_t value)
{
    if (!mqtt)
        return;

    // Create LDRData message
    transporter_LDRData ldrData = transporter_LDRData_init_zero;
    ldrData.id = id;
    ldrData.value = value;

    // Serialize the message
    uint8_t buffer[64];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    bool status = pb_encode(&stream, transporter_LDRData_fields, &ldrData);
    if (!status)
    {
        Serial.print("SensorManager: Failed to encode LDR data: ");
        Serial.println(PB_GET_ERROR(&stream));
        return;
    }

    // Construct the MQTT topic
    String topic = "arduino/" + deviceId + "/ldr";

    // Publish the message
    mqtt->publish(topic.c_str(), buffer, stream.bytes_written);
}

/**
 * @brief Publish relay state to MQTT topic
 */
void SensorManager::publishRelayState(uint8_t type, uint8_t port, uint8_t state)
{
    if (!mqtt)
        return;

    // Create RelayState message
    transporter_RelayState relayState = transporter_RelayState_init_zero;
    relayState.type = (transporter_RelayType)type;
    relayState.port = port;
    relayState.state = (transporter_RelayStateType)state;

    // Serialize the message
    uint8_t buffer[64];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    bool status = pb_encode(&stream, transporter_RelayState_fields, &relayState);
    if (!status)
    {
        Serial.print("SensorManager: Failed to encode relay state: ");
        Serial.println(PB_GET_ERROR(&stream));
        return;
    }

    // Construct the MQTT topic
    String topic = "arduino/" + deviceId + "/relay";

    // Publish the message
    mqtt->publish(topic.c_str(), buffer, stream.bytes_written);
}