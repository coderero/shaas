#ifndef CLIMATE_H
#define CLIMATE_H

#include <modules/mux.h>

#include <DHT.h>
#include <Air_Quality_Sensor.h>

/**
 * @class Climate
 * @brief Provides environmental sensor readings including temperature, humidity, and air quality index.
 *
 * This class uses a DHT22 sensor and an analog air quality sensor. Both sensors are accessed through
 * a Mux (multiplexer) instance to allow shared use of IO pins.
 */
class Climate
{
private:
    int id;                ///< Unique identifier for the climate module
    int d_port;            ///< Mux channel for the DHT sensor
    uint8_t a_port;        ///< Mux channel for the air quality sensor
    int signal_pin;        ///< Digital GPIO pin connected to the DHT sensor
    float temperature;     ///< Last recorded temperature (°C)
    float humidity;        ///< Last recorded relative humidity (%)
    int air_quality_index; ///< Last recorded air quality index (integer value)

    Mux *mux;                            ///< Pointer to a shared Mux instance
    DHT *dht_sensor;                     ///< Pointer to DHT sensor instance
    AirQualitySensor air_quality_sensor; ///< Instance of the air quality sensor

public:
    /**
     * @brief Default constructor. Use `init()` to set up sensors.
     */
    Climate();

    /**
     * @brief Destructor. Frees dynamically allocated memory.
     */
    ~Climate();

    /**
     * @brief Initializes the climate module with given parameters.
     *
     * @param d_port Digital mux channel for DHT sensor
     * @param a_port Analog mux channel for air quality sensor
     * @param signal_pin GPIO pin used for DHT communication
     * @param mux Pointer to an existing Mux instance
     * @return true if both sensors initialized successfully, false otherwise
     */
    bool init(int id, int d_port, const uint8_t &a_port, int signal_pin, Mux *mux);

    /**
     * @brief Reads and updates the sensor values (temperature, humidity, AQI).
     *
     * The function switches the mux to the appropriate channels before reading the sensors.
     * It stores the latest readings in class members.
     */
    void read_climate_data();

    /**
     * @brief Returns the last recorded temperature in Celsius.
     * @return float temperature in °C
     */
    float get_temperature() const;

    /**
     * @brief Returns the last recorded humidity as a percentage.
     * @return float relative humidity in %
     */
    float get_humidity() const;

    /**
     * @brief Returns the last recorded air quality index.
     * @return int AQI value
     */
    int get_air_quality_index() const;

    /**
     * @brief Returns the unique identifier for this climate module.
     * @return int ID of the climate module
     */
    int get_id() const;

    /**
     * @brief Sets the DHT sensor mux channel.
     * @param d_port New mux channel for DHT
     */
    void set_d_port(int d_port);

    /**
     * @brief Gets the current mux channel used for DHT sensor.
     * @return int mux channel
     */
    int get_d_port() const;

    /**
     * @brief Sets the air quality sensor mux channel.
     * @param a_port New mux channel for air quality sensor
     */
    void set_a_port(int a_port);

    /**
     * @brief Gets the current mux channel used for air quality sensor.
     * @return int mux channel
     */
    int get_a_port() const;
};

#endif // CLIMATE_H
