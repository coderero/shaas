#include <communication/serial_module.h>

/**
 * @brief Default constructor
 */
SerialModule::SerialModule() : serialPort(nullptr), initialized(false)
{
}

/**
 * @brief Destructor
 */
SerialModule::~SerialModule()
{
    // No dynamic memory to clean up
}

/**
 * @brief Initializes the serial module with the specified hardware serial port
 * @param port Reference to the HardwareSerial port to use
 * @return true if initialization was successful
 */
bool SerialModule::init(Stream &port)
{
    serialTransfer.begin(port);
    initialized = true;

    return true;
}

/**
 * @brief Checks if data is available to receive
 * @return true if data is available
 */
bool SerialModule::available()
{
    if (!initialized)
        return false;

    // Check if data is available and handle it
    if (serialTransfer.available())
    {
        return true;
    }

    // Check for packet parsing errors
    if (serialTransfer.status < 0)
    {
        Serial.print("SerialTransfer ERROR: ");
        Serial.println(serialTransfer.status);
        return false;
    }

    return false;
}