#ifndef CLIMATE_H
#define CLIMATE_H

#include <mux.h>
#include <DHT.h>
#include <Air_Quality_Sensor.h>

/**
 * @brief Climate class for reading temperature, humidity, and air quality using a multiplexer
 */
class Climate
{
private:
    int dport;           ///< Digital port (mux channel) for DHT sensor
    int aport;           ///< Analog port (mux channel) for air quality sensor
    float temperature;   ///< Latest temperature reading
    float humidity;      ///< Latest humidity reading
    int airQualityIndex; ///< Latest air quality index reading

    Mux *m;                              ///< Pointer to Mux instance for channel selection
    DHT dht;                             ///< DHT sensor instance (for temperature and humidity)
    AirQualitySensor air_quality_sensor; ///< Air quality sensor instance

public:
    /**
     * @brief Constructor for Climate class
     * @param dport Mux channel for digital DHT sensor
     * @param aport Mux channel for analog air quality sensor
     * @param signal_pin Pin used for reading DHT data
     * @param mux Pointer to initialized Mux object
     */
    Climate(int dport, const uint8_t &aport, int signal_pin, Mux *mux);

    /**
     * @brief Destructor for Climate class
     */
    ~Climate();

    /**
     * @brief Initializes the sensors (DHT and air quality)
     * @return true if initialization successful, false otherwise
     */
    bool init();

    /**
     * @brief Reads and updates climate data (temperature, humidity, air quality index)
     */
    void readClimateData();

    /**
     * @brief Returns the latest temperature reading
     * @return Temperature in Celsius
     */
    float getTemperature() const;

    /**
     * @brief Returns the latest humidity reading
     * @return Relative humidity in percentage
     */
    float getHumidity() const;

    /**
     * @brief Returns the latest air quality index reading
     * @return Integer AQI value
     */
    int getAirQualityIndex() const;

    /**
     * @brief Sets the DHT sensor mux channel
     * @param dport Channel to set
     */
    void setDPort(int dport);

    /**
     * @brief Gets the DHT sensor mux channel
     * @return Digital mux channel
     */
    int getDPort() const;

    /**
     * @brief Sets the air quality sensor mux channel
     * @param aport Channel to set
     */
    void setAPort(int aport);

    /**
     * @brief Gets the air quality sensor mux channel
     * @return Analog mux channel
     */
    int getAPort() const;
};

#endif
