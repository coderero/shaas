/**
 * @file IRFIDAuthenticator.h
 * @brief Interface for RFID authentication mechanism.
 *
 * This interface is designed to abstract the authentication logic
 * for RFID-based systems. It allows for flexible implementations,
 * such as MQTT-based verification or local whitelist checks,
 * without tightly coupling the code to a specific method.
 */

#ifndef IRFIDAUTHENTICATOR_H
#define IRFIDAUTHENTICATOR_H

#include <Arduino.h>

/**
 * @class IRFIDAuthenticator
 * @brief Interface class for RFID authentication.
 *
 * Classes implementing this interface must provide a method to
 * determine if the most recently scanned RFID UID is authenticated.
 */
class IRFIDAuthenticator
{
public:
    /**
     * @brief Verifies if the given UID is authorized.
     * @param uid Pointer to UID byte array
     * @param length Length of the UID array
     * @return true if authenticated, false otherwise
     */
    virtual bool is_authenticated(const byte *uid, byte length) = 0;

    /**
     * @brief Registers a new UID for authentication.
     * @param uid Pointer to UID byte array
     * @param length Length of the UID array
     * @return true if registration was successful, false otherwise
     */
    virtual bool register_uid(const byte *uid, byte length) = 0;

    /**
     * @brief Virtual destructor.
     *
     * Ensures derived class destructors are properly called.
     */
    virtual ~IRFIDAuthenticator() {};
};

#endif // IRFIDAUTHENTICATOR_H
