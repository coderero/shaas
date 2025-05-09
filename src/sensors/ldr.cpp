#include <sensors/ldr.h>

/**
 * @brief Initializes the LDR sensor with the specified pin and threshold.
 *
 * This function sets the analog pin used for reading light levels and defines the
 * threshold value for determining light detection. It also sets the analog pin mode to INPUT.
 *
 * @param pin The analog pin to which the LDR sensor is connected..
 */
void LDR::init(int id, uint8_t pin)
{
    _analog_pin = pin;
    ;
    pinMode(_analog_pin, INPUT);
}

/**
 * @brief Returns the unique identifier for this LDR sensor.
 *
 * @return The unique identifier of the LDR sensor.
 */
int LDR::get_id() const
{
    return id;
}

/**
 * @brief Reads the current analog value from the LDR sensor.
 *
 * @return The analog value representing the light intensity from the sensor.
 */
int LDR::read()
{
    return analogRead(_analog_pin);
}
