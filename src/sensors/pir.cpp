#include <sensors/pir.h>

#include <Arduino.h>

/**
 * @brief Sets the GPIO pin used to read PIR sensor data.
 * @param port The digital pin number where the PIR sensor is connected.
 */
void PIR::set_port(int port)
{
    _port = port;
    pinMode(_port, INPUT); // Ensure the pin is set as input
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
    movement = (digitalRead(_port) == HIGH);
}
