#ifndef RELAY_CONTROL_H
#define RELAY_CONTROL_H

#include <communication/serial_module.h>

// Constants for relay commands
#define TOGGLE_RELAY 1
#define GET_RELAY_STATE 2

// Constants for relay types
#define LOW_DUTY 1   // 10A, 4 switches
#define HEAVY_DUTY 2 // 30A, 2 switches

/**
 * @struct RelayCommand
 * @brief Structure for relay control commands sent over serial
 */
struct __attribute__((packed)) RelayCommand
{
    uint8_t command; // Command type (1 = TOGGLE_RELAY, 2 = GET_RELAY_STATE)
    uint8_t type;    // Relay type (1 = LOW_DUTY, 2 = HEAVY_DUTY)
    uint8_t port;    // Port number (starting from 1)
    uint8_t state;   // Desired state (0 = OFF, 1 = ON)
};

/**
 * @class RelayControl
 * @brief Manages control of external relay modules via serial communication
 */
class RelayControl
{
private:
    SerialModule *serialModule;
    bool initialized = false;

public:
    /**
     * @brief Default constructor
     */
    RelayControl();

    /**
     * @brief Destructor
     */
    ~RelayControl();

    /**
     * @brief Initializes the relay control module
     * @param serialModule Pointer to a SerialModule instance
     * @return true if initialization was successful
     */
    bool init(SerialModule *serialModule);

    /**
     * @brief Toggles a relay's state
     * @param type Relay type (LOW_DUTY or HEAVY_DUTY)
     * @param port Port number (1-based index)
     * @param state Desired state (0 = OFF, 1 = ON)
     * @return true if command was sent successfully
     */
    bool toggleRelay(uint8_t type, uint8_t port, uint8_t state);

    /**
     * @brief Requests the current state of a relay
     * @param type Relay type (LOW_DUTY or HEAVY_DUTY)
     * @param port Port number (1-based index)
     * @return true if request was sent successfully
     */
    bool getRelayState(uint8_t type, uint8_t port);

    /**
     * @brief Handles incoming relay state responses
     * @return true if a response was handled
     */
    bool handleResponses();
};

#endif // RELAY_CONTROL_H