#include <sensors/climate.h>

#include <Arduino.h>

/**
 * @brief Constructs a Climate instance with default values.
 */
Climate::Climate()
    : d_port(-1), a_port(-1), signal_pin(-1),
      temperature(0.0f), humidity(0.0f), air_quality_index(0),
      mux(nullptr), dht_sensor(nullptr),
      air_quality_sensor(A0) // Default analog pin
{
}

/**
 * @brief Destroys the Climate instance and releases DHT sensor memory.
 */
Climate::~Climate()
{
    if (dht_sensor)
    {
        delete dht_sensor;
        dht_sensor = nullptr;
    }
}

/**
 * @brief Initializes the Climate module.
 *
 * This function sets mux ports, pins, and allocates the DHT sensor.
 * @param d_port Digital mux channel for DHT sensor
 * @param a_port Analog mux channel for air quality sensor
 * @param signal_pin GPIO pin used for DHT data
 * @param mux Pointer to a Mux object
 * @return true if air quality sensor initializes correctly
 */
bool Climate::init(int id, int d_port, const uint8_t &a_port, int signal_pin, Mux *mux)
{
    this->id = id;
    this->d_port = d_port;
    this->a_port = a_port;
    this->signal_pin = signal_pin;
    this->mux = mux;

    if (dht_sensor)
    {
        delete dht_sensor;
    }

    dht_sensor = new DHT(signal_pin, DHT22);
    dht_sensor->begin();

    return air_quality_sensor.init();
}

/**
 * @brief Reads temperature, humidity, and air quality values.
 *
 * Uses mux to select proper channels before reading sensors.
 * Stores the values for later access via getters.
 */
void Climate::read_climate_data()
{
    // Read DHT sensor via mux
    mux->channel(d_port);
    pinMode(mux->getSignalPin(), INPUT_PULLUP);
    temperature = dht_sensor->readTemperature();
    humidity = dht_sensor->readHumidity();

    // Read air quality sensor directly
    air_quality_index = air_quality_sensor.getValue();

    // Handle invalid sensor readings
    if (isnan(temperature) || isnan(humidity) || air_quality_index < 0)
    {
        Serial.println("Failed to read from sensors!");
    }
}

/**
 * @brief Returns the latest temperature reading.
 * @return Temperature in Celsius
 */
float Climate::get_temperature() const
{
    return temperature;
}

/**
 * @brief Returns the latest humidity reading.
 * @return Relative humidity in percentage
 */
float Climate::get_humidity() const
{
    return humidity;
}

/**
 * @brief Returns the latest air quality index.
 * @return AQI integer value
 */
int Climate::get_air_quality_index() const
{
    return air_quality_index;
}

/**
 * @brief Returns the unique identifier for this climate module.
 * @return ID of the climate module
 */
int Climate::get_id() const
{
    return id;
}

/**
 * @brief Updates the mux channel used for DHT sensor.
 * @param d_port New digital port
 */
void Climate::set_d_port(int d_port)
{
    this->d_port = d_port;
}

/**
 * @brief Returns the mux channel used for DHT sensor.
 * @return Digital port
 */
int Climate::get_d_port() const
{
    return d_port;
}

/**
 * @brief Updates the mux channel used for air quality sensor.
 * @param a_port New analog port
 */
void Climate::set_a_port(int a_port)
{
    this->a_port = a_port;
}

/**
 * @brief Returns the mux channel used for air quality sensor.
 * @return Analog port
 */
int Climate::get_a_port() const
{
    return a_port;
}
