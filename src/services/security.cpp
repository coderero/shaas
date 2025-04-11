/**
 * @file security.cpp
 * @brief Implements the Security class for RFID-based access control system.
 *
 * This source file defines the methods of the Security class. The class is
 * responsible for coordinating the RFID reader, lock control, and the
 * authentication mechanism to create a secure access system.
 */

#include <services/security.h>

/**
 * @brief Constructs the Security object with null pointers.
 *
 * The constructor initializes all internal module pointers (RFID, Lock,
 * and Authenticator) to nullptr. They are expected to be initialized
 * via the `init()` method.
 */
Security::Security()
    : rfid(nullptr),
      lock(nullptr),
      authenticator(nullptr)
{
}

/**
 * @brief Destructs the Security object and releases allocated memory.
 *
 * This destructor ensures that dynamically allocated resources for the RFID,
 * Lock, and Authenticator modules are properly deallocated to prevent memory leaks.
 */
Security::~Security()
{
    if (rfid)
    {
        delete rfid;
        rfid = nullptr;
    }
    if (lock)
    {
        delete lock;
        lock = nullptr;
    }

    if (authenticator)
    {
        delete authenticator;
        authenticator = nullptr;
    }
}

/**
 * @brief Initializes the Security system with the provided modules.
 *
 * This method initializes the RFID reader and associates it with the
 * provided authenticator. It also initializes the lock module.
 *
 * @param authenticator Pointer to an IRFIDAuthenticator implementation.
 * @return true if initialization is successful, false otherwise.
 */
bool Security::init(IRFIDAuthenticator *authenticator)
{
    if (!authenticator)
    {
        return false;
    }

    this->authenticator = authenticator;

    // Initialize RFID module
    rfid = new RFID();
    if (!rfid->init(authenticator))
    {
        delete rfid;
        rfid = nullptr;
        return false;
    }

    // Initialize Lock module
    lock = new Lock();
    if (!lock->init())
    {
        delete lock;
        lock = nullptr;
        return false;
    }

    return true;
}

/**
 * @brief Reads an RFID card and checks its authentication status.
 *
 * This method attempts to read a card using the RFID module. If a card is detected,
 * it validates the UID using the provided authenticator. On successful authentication,
 * the lock is unlocked.
 *
 * @return true if the card is read and authenticated successfully, false otherwise.
 */
bool Security::read_card()
{
    if (!rfid || !authenticator)
    {
        return false;
    }

    if (rfid->read_card())
    {
        lock->unlock();
        return true;
    }
    return false;
}

/**
 * @brief Registers the current RFID card with the authenticator.
 *
 * This method attempts to register the currently read card UID into the system
 * using the authenticator module. If registration is successful, the lock is locked
 * to secure access until future authenticated entry.
 *
 * @return true if UID registration is successful, false otherwise.
 */
bool Security::register_uid()
{
    if (!rfid || !authenticator)
    {
        return false;
    }

    if (rfid->register_card())
    {
        // If the card is registered, lock the lock
        lock->lock();
        return true;
    }
    return false;
}
