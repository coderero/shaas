#include <Arduino.h>
#include <climate.h>
#include <mux.h>
#include <DHT.h>
#include <Air_Quality_Sensor.h>

/**
 * @brief Constructor for Climate class
 * Initializes DHT and Air Quality Sensor with given mux channels and signal pin
 * @param dport Mux channel for DHT sensor
 * @param aport Mux channel for air quality sensor
 * @param signal_pin Pin connected to DHT sensor
 * @param mux Pointer to Mux instance for channel selection
 */
Climate::Climate(int dport, const uint8_t &aport, int signal_pin, Mux *mux)
    : dport(dport),
      aport(aport),
      m(mux),
      dht(signal_pin, DHT22), // Initialize DHT with signal pin
      air_quality_sensor(A0)  // Default pin for air quality sensor
{
}

/**
 * @brief Destructor for Climate class
 * No dynamic memory used, so default is fine
 */
Climate::~Climate()
{
}

/**
 * @brief Initializes both sensors (DHT and air quality)
 * @return true if both sensors initialized successfully
 */
bool Climate::init()
{
    dht.begin(); // Start DHT sensor

    if (!air_quality_sensor.init())
    {
        return false; // Air quality sensor failed to init
    }

    return true;
}

/**
 * @brief Reads and stores temperature, humidity, and air quality index
 * Uses mux to select correct channels before reading sensors
 */
void Climate::readClimateData()
{
    // Select DHT channel on mux
    m->channel(dport);
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();

    // Select air quality channel on mux (if needed)
    // NOTE: If your air quality sensor shares a pin or needs separate muxing,
    // you could do m->channel(aport); here. Otherwise, it's assumed wired directly.

    airQualityIndex = air_quality_sensor.getValue();

    // Error handling for bad readings
    if (isnan(temperature) || isnan(humidity) || airQualityIndex < 0)
    {
        Serial.println("Failed to read from sensors!");
        return;
    }
}

/**
 * @brief Gets the last recorded temperature
 * @return Temperature in Celsius
 */
float Climate::getTemperature() const
{
    return temperature;
}

/**
 * @brief Gets the last recorded humidity
 * @return Relative humidity in %
 */
float Climate::getHumidity() const
{
    return humidity;
}

/**
 * @brief Gets the last recorded air quality index
 * @return Integer AQI value
 */
int Climate::getAirQualityIndex() const
{
    return airQualityIndex;
}

/**
 * @brief Sets the mux channel for the DHT sensor
 * @param d New digital port (mux channel)
 */
void Climate::setDPort(int d)
{
    dport = d;
}

/**
 * @brief Gets the current mux channel used for DHT sensor
 * @return Digital port
 */
int Climate::getDPort() const
{
    return dport;
}

/**
 * @brief Sets the mux channel for the air quality sensor
 * @param a New analog port
 */
void Climate::setAPort(int a)
{
    aport = a;
}

/**
 * @brief Gets the current mux channel for the air quality sensor
 * @return Analog port
 */
int Climate::getAPort() const
{
    return aport;
}
