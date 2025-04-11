#if !defined(LOCK_H)
#define LOCK_H

#include <modules/mux.h>

#include <Servo.h>

/**
 * @brief Enumeration representing the lock status.
 *
 * - LOCK: The system is in a locked state.
 * - UNLOCK: The system is in an unlocked state.
 */
typedef enum
{
    LOCK,
    UNLOCK
} lock_status;

/**
 * @brief Lock class for controlling a servo-based locking mechanism.
 *
 * This class encapsulates the functionality for initializing, locking,
 * unlocking, and checking the status of a servo-controlled lock.
 */
class Lock
{
private:
    lock_status _lock_status = LOCK; ///< Current lock status

    Servo _lock_servo; ///< Servo object used to control the physical lock

public:
    /**
     * @brief Constructor for the Lock class.
     * Initializes internal lock status to LOCK.
     */
    Lock();

    /**
     * @brief Destructor for the Lock class.
     * Cleans up resources if needed.
     */
    ~Lock();

    /**
     * @brief Initializes the lock mechanism by attaching the servo.
     *
     * This method sets up the servo motor for controlling the lock.
     *
     * @return true if initialization was successful.
     */
    bool init();

    /**
     * @brief Engages the lock by setting the servo to the locked position.
     *
     * @return true if locking was successful.
     */
    bool lock();

    /**
     * @brief Unlocks the lock by rotating the servo.
     * Automatically re-locks after 5 seconds.
     *
     * @return true if unlocking was successful.
     */
    bool unlock();

    /**
     * @brief Gets the current lock status.
     *
     * @return The current lock status (LOCK or UNLOCK).
     */
    lock_status get_lock_status();
};

#endif // LOCK_H
