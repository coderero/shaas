#include <devices/relay_control.h>

/**
 * @brief Default constructor
 */
RelayControl::RelayControl() : serialModule(nullptr), initialized(false)
{
}

/**
 * @brief Destructor
 */
RelayControl::~RelayControl()
{
    // SerialModule is externally owned, don't delete it
}

/**
 * @brief Initializes the relay control module
 * @param module Pointer to a SerialModule instance
 * @return true if initialization was successful
 */
bool RelayControl::init(SerialModule *module)
{
    if (!module)
        return false;

    serialModule = module;
    initialized = true;

    Serial.println("RelayControl: Initialized successfully");
    return true;
}

/**
 * @brief Toggles a relay's state
 * @param type Relay type (LOW_DUTY or HEAVY_DUTY)
 * @param port Port number (1-based index)
 * @param state Desired state (0 = OFF, 1 = ON)
 * @return true if command was sent successfully
 */
bool RelayControl::toggleRelay(uint8_t type, uint8_t port, uint8_t state)
{
    if (!initialized)
        return false;

    RelayCommand cmd;
    cmd.command = TOGGLE_RELAY;
    cmd.type = type;
    cmd.port = port;
    cmd.state = state;
    return serialModule->sendObject(cmd);
}

/**
 * @brief Requests the current state of a relay
 * @param type Relay type (LOW_DUTY or HEAVY_DUTY)
 * @param port Port number (1-based index)
 * @return true if request was sent successfully
 */
bool RelayControl::getRelayState(uint8_t type, uint8_t port)
{
    if (!initialized)
        return false;

    RelayCommand cmd;
    cmd.command = GET_RELAY_STATE;
    cmd.type = type;
    cmd.port = port;
    cmd.state = 0; // Not used for GET_RELAY_STATE

    Serial.print("RelayControl: Getting state for relay type=");
    Serial.print(type);
    Serial.print(" port=");
    Serial.println(port);

    return serialModule->sendObject(cmd);
}

/**
 * @brief Handles incoming relay state responses
 * @return true if a response was handled
 */
bool RelayControl::handleResponses()
{
    if (!initialized || !serialModule->available())
        return false;

    RelayCommand response;
    if (serialModule->receiveObject(response))
    {
        // Process the response based on command type
        if (response.command == GET_RELAY_STATE)
        {
            Serial.print("RelayControl: Received state for relay type=");
            Serial.print(response.type);
            Serial.print(" port=");
            Serial.print(response.port);
            Serial.print(" state=");
            Serial.println(response.state);
            return true;
        }

        return true;
    }

    return false;
}