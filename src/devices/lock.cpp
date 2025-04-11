#include <devices/lock.h>

#include <Arduino.h>
#include <Servo.h>

/**
 * @brief Constructor for the Lock class.
 * Initializes the internal lock status to LOCK.
 */
Lock::Lock()
{
    _lock_status = LOCK;
}

/**
 * @brief Destructor for the Lock class.
 * Cleanup operations can be added if needed.
 */
Lock::~Lock()
{
    // Destructor implementation (if needed)
}

/**
 * @brief Initializes the lock mechanism by attaching the servo.
 * This method sets up the servo motor for controlling the lock.
 * @return true if initialization is successful.
 */
bool Lock::init()
{
    _lock_servo.attach(6); // Attach the servo to pin 9
    _lock_servo.write(0);  // Set initial position to "locked"
    delay(1000);           // Allow time for servo to reach position
    return true;
}

/**
 * @brief Locks the mechanism by rotating the servo to the locked position.
 * @return true if the operation was successfully initiated.
 */
bool Lock::lock()
{
    _lock_servo.write(0); // Rotate servo to "locked" position
    delay(1000);          // Allow time for movement
    _lock_status = LOCK;  // Update status
    return true;
}

/**
 * @brief Unlocks the mechanism by rotating the servo to the unlocked position.
 * After unlocking, it automatically relocks after 5 seconds.
 * @return true if the operation was successfully initiated.
 */
bool Lock::unlock()
{
    _lock_servo.write(100); // Rotate servo to "unlocked" position
    delay(3000);            // Keep unlocked for 5 seconds
    _lock_servo.write(0);   // Relock automatically
    delay(1000);            // Allow time for relocking
    _lock_status = UNLOCK;  // Temporarily update status (consider status logic)
    return true;
}

/**
 * @brief Gets the current status of the lock.
 * @return lock_status enum indicating LOCK or UNLOCK state.
 */
lock_status Lock::get_lock_status()
{
    return _lock_status;
}
