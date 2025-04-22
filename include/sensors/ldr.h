#ifndef LDR_H
#define LDR_H

#include <Arduino.h>

/**
 * @class LDR
 * @brief Represents a Light Dependent Resistor (LDR) sensor for detecting ambient light levels.
 */
class LDR
{
private:
    int id;              ///< Unique identifier for the LDR sensor.
    uint8_t _analog_pin; ///< Analog pin connected to the LDR.

public:
    /**
     * @brief Constructs an LDR sensor object.
     */
    LDR() : _analog_pin(A0) {} // Default pin and threshold

    /**
     * @brief Initializes the LDR pin as input.
     * @param pin The analog pin connected to the LDR.
     */
    void init(int id, uint8_t pin);

    /**
     * @brief Returns the unique identifier for this LDR sensor.
     * @return The unique identifier of the LDR sensor.
     */
    int get_id() const;

    /**
     * @brief Reads the current analog value from the LDR.
     * @return The raw analog value from the LDR.
     */
    int read();
};

#endif // LDR_H
