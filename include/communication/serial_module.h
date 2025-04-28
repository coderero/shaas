#ifndef SERIAL_MODULE_H
#define SERIAL_MODULE_H

#include <Arduino.h>
#include <SerialTransfer.h>

/**
 * @class SerialModule
 * @brief Manages serial communication using the SerialTransfer library.
 *
 * This class encapsulates SerialTransfer functionality to facilitate structured
 * data transfer between devices over serial connections.
 */
class SerialModule
{
private:
    SerialTransfer serialTransfer;
    HardwareSerial *serialPort;
    bool initialized = false;

public:
    /**
     * @brief Default constructor
     */
    SerialModule();

    /**
     * @brief Destructor
     */
    ~SerialModule();

    /**
     * @brief Initializes the serial module with the specified hardware serial port
     * @param port Pointer to the HardwareSerial port to use
     * @return true if initialization was successful
     */
    bool init(Stream &port);

    /**
     * @brief Sends an object via the SerialTransfer interface
     * @param obj Reference to the object to send
     * @param size Size of the object in bytes
     * @return true if send was successful
     */
    template <typename T>
    bool sendObject(const T &obj)
    {
        if (!initialized)
            return false;

        uint16_t sendSize = 0;
        sendSize = serialTransfer.txObj(obj, sendSize);
        return serialTransfer.sendData(sendSize);
    }

    /**
     * @brief Checks if data is available to receive
     * @return true if data is available
     */
    bool available();

    /**
     * @brief Receives an object via the SerialTransfer interface
     * @param obj Reference to store the received object
     * @return true if receive was successful
     */
    template <typename T>
    bool receiveObject(T &obj)
    {
        if (!initialized || !available())
            return false;

        uint16_t recSize = 0;
        serialTransfer.rxObj(obj, recSize);
        return true;
    }
};

#endif // SERIAL_MODULE_H
