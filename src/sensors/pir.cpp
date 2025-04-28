#include <sensors/pir.h>

#include <Arduino.h>

/**
 * @brief Initializes the PIR sensor with a Mux object.
 * @param id Unique identifier for the PIR sensor.
 * @param mux Pointer to the Mux object for channel selection.
 */
void PIR::init(int id, Mux *mux)
{
    this->id = id;
    _mux = mux;
}

/**
 * @brief Returns the unique identifier for this PIR sensor.
 * @return The unique identifier of the PIR sensor.
 */
int PIR::get_id() const
{
    return id;
}

/**
 * @brief Sets the GPIO pin used to read PIR sensor data.
 * @param port The digital pin number where the PIR sensor is connected.
 */
void PIR::set_port(int port)
{
    _port = port;
}

/**
 * @brief Gets the GPIO pin currently assigned to the PIR sensor.
 * @return The pin number assigned to the sensor.
 */
int PIR::get_port()
{
    return _port;
}

/**
 * @brief Returns the last recorded motion state from the PIR sensor.
 * @return True if motion was detected; false otherwise.
 */
bool PIR::get_movement()
{
    detect_movement();
    return movement;
}

/**
 * @brief Reads the sensor and updates the motion detection status.
 *
 * This method performs a digital read on the assigned pin.
 * If the sensor output is HIGH, motion is detected and the internal state is updated.
 */
void PIR::detect_movement()
{
    if (_mux)
    {
        _mux->m_mode(DIGITAL);
        _mux->s_mode(MUX_INPUT);
        _mux->channel(_port);
        movement = _mux->read() == HIGH;
    }
    else
    {
        pinMode(_port, INPUT);
        movement = digitalRead(_port) == HIGH;
    }
}
