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
    uint16_t _threshold; ///< Threshold value to determine light/no-light condition.

public:
    /**
     * @brief Constructs an LDR sensor object.
     */
    LDR() : _analog_pin(A0), _threshold(512) {} // Default pin and threshold

    /**
     * @brief Initializes the LDR pin as input.
     * @param pin The analog pin connected to the LDR.
     * @param threshold The threshold value for light detection.
     */
    void init(int id, uint8_t pin, uint16_t threshold);

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

    /**
     * @brief Returns the current threshold used for detecting light.
     * @return The threshold value.
     */
    int get_threshold();
};

#endif // LDR_H
