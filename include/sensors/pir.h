#ifndef PIR_H
#define PIR_H

/**
 * @class PIR
 * @brief A class to handle Passive Infrared (PIR) motion sensor input.
 *
 * This class encapsulates functionality to read motion data from a PIR sensor
 * connected to a digital input pin on an Arduino-compatible board.
 */
class PIR
{
private:
    int _port;     ///< GPIO pin connected to the PIR sensor
    bool movement; ///< Boolean flag indicating whether motion is detected

public:
    /**
     * @brief Sets the digital pin connected to the PIR sensor.
     * @param port The GPIO pin number.
     */
    void set_port(int port);

    /**
     * @brief Gets the currently assigned digital pin number for the PIR sensor.
     * @return The GPIO pin number.
     */
    int get_port();

    /**
     * @brief Retrieves the current motion detection state.
     * @return True if motion is detected, false otherwise.
     */
    bool get_movement();

    /**
     * @brief Reads the PIR sensor and updates the internal motion state.
     *
     * This method should be called frequently (e.g., in the main loop)
     * to monitor motion status in real-time.
     */
    void detect_movement();
};

#endif // PIR_H
