/**
 * @file security.h
 * @brief Defines the Security class for handling RFID-based access control.
 *
 * This file contains the declaration of the `Security` class, which integrates
 * the RFID module, locking mechanism, and an authentication interface.
 * The class provides functions for initializing the system, reading RFID cards,
 * and registering new UID entries.
 *
 * @author
 * Mohit Sharma
 * @date
 * April 11, 2025
 */

#ifndef SECURITY_H
#define SECURITY_H

#include <modules/rfid.h>
#include <devices/lock.h>
#include <interfaces/IRFIDAuthenticator.h>

/**
 * @class Security
 * @brief Manages security operations using RFID and locking mechanisms.
 *
 * The `Security` class provides a high-level interface to integrate the RFID reader,
 * a physical lock, and an authentication system. It enables operations such as
 * card reading and UID registration while abstracting the underlying hardware
 * and validation mechanisms.
 */
class Security
{
private:
    RFID *rfid;                        ///< Pointer to the RFID module.
    Lock *lock;                        ///< Pointer to the lock module.
    IRFIDAuthenticator *authenticator; ///< Pointer to the RFID authentication interface.

public:
    /**
     * @brief Default constructor for the Security class.
     */
    Security();

    /**
     * @brief Destructor for the Security class.
     */
    ~Security();

    /**
     * @brief Initializes the security system with the given authenticator.
     *
     * This method sets up internal references and prepares the system
     * for operation.
     *
     * @param authenticator A pointer to an implementation of IRFIDAuthenticator.
     * @return true if initialization is successful, false otherwise.
     */
    bool init(IRFIDAuthenticator *authenticator);

    /**
     * @brief Reads a card from the RFID module and performs authentication.
     *
     * This method checks for a card, retrieves the UID, and delegates
     * the authentication to the configured authenticator.
     *
     * @return true if authentication is successful, false otherwise.
     */
    bool read_card();

    /**
     * @brief Registers a UID using the connected authenticator.
     *
     * This method enables the system to register new UIDs into the authenticator
     * (e.g., storing them in a database or in memory).
     *
     * @return true if UID is successfully registered, false otherwise.
     */
    bool register_uid();
};

#endif // SECURITY_H
