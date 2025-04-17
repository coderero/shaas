#ifndef PIR_H
#define PIR_H

#include <modules/mux.h>

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
    int id;        ///< Unique identifier for the PIR sensor
    int _port;     ///< GPIO pin connected to the PIR sensor
    bool movement; ///< Boolean flag indicating whether motion is detected

    Mux *_mux; ///< Pointer to the Mux object for channel selection

public:
    /**
     * @brief Default constructor for the PIR class.
     */
    PIR()
        : id(-1), _port(-1), movement(false), _mux(nullptr) {} // Default values
    /**
     * @brief Destructor for the PIR class.
     */
    ~PIR() {};

    /**
     * @brief Initializes the PIR sensor with a Mux object.
     * @param id Unique identifier for the PIR sensor.
     * @param mux Pointer to the Mux object for channel selection.
     */
    void init(int id, Mux *mux);

    /**
     * @brief Sets the unique identifier for the PIR sensor.
     * @return The unique identifier of the PIR sensor.
     */
    int get_id() const;

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
