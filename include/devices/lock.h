#ifndef LOCK_H
#define LOCK_H

#include <Arduino.h>
#include <Servo.h>

/**
 * @brief Enumeration representing the lock status.
 */
typedef enum
{
    LOCK,
    UNLOCKING,
    UNLOCKED,
    RELOCKING
} lock_status;

/**
 * @class Lock
 * @brief Manages a servo-based locking mechanism with non-blocking behavior.
 */
class Lock
{
private:
    Servo _lock_servo;                ///< Servo object used to control the physical lock
    uint32_t _last_action_time;       ///< Timestamp for last lock/unlock action
    uint32_t _unlock_duration = 3000; ///< Duration in milliseconds to keep unlocked
    lock_status _lock_status;         ///< Current lock state
    bool _action_pending;             ///< Flag to track if a transition is in progress

public:
    /**
     * @brief Constructs the Lock object with default locked status.
     */
    Lock();

    /**
     * @brief Initializes the servo pin and locks initially.
     * @return true if initialization succeeds.
     */
    bool init();

    /**
     * @brief Non-blocking call to begin unlocking process.
     * @return true if transition initiated.
     */
    bool unlock();

    /**
     * @brief Immediately locks the servo.
     * @return true if locked successfully.
     */
    bool lock();

    /**
     * @brief Non-blocking update handler to manage timing-based relocking.
     */
    void handle();

    /**
     * @brief Returns current lock status.
     */
    lock_status get_lock_status();
};

#endif // LOCK_H
