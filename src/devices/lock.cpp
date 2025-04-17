#include <devices/lock.h>

Lock::Lock() : _lock_status(LOCK), _action_pending(false), _last_action_time(0) {}

bool Lock::init()
{
    _lock_servo.attach(6);
    _lock_servo.write(0); // Locked position
    _lock_status = LOCK;
    return true;
}

bool Lock::unlock()
{
    if (_lock_status == LOCK && !_action_pending)
    {
        _lock_servo.write(100); // Unlocked position
        _lock_status = UNLOCKING;
        _last_action_time = millis();
        _action_pending = true;
        return true;
    }
    return false;
}

bool Lock::lock()
{
    _lock_servo.write(0); // Locked position
    _lock_status = LOCK;
    _action_pending = false;
    return true;
}

void Lock::handle()
{
    if (_lock_status == UNLOCKING && (millis() - _last_action_time >= 1000))
    {
        _lock_status = UNLOCKED;
        _last_action_time = millis();
    }

    else if (_lock_status == UNLOCKED && (millis() - _last_action_time >= _unlock_duration))
    {
        _lock_servo.write(0); // Lock back
        _lock_status = RELOCKING;
        _last_action_time = millis();
    }

    else if (_lock_status == RELOCKING && (millis() - _last_action_time >= 1000))
    {
        _lock_status = LOCK;
        _action_pending = false;
    }
}

lock_status Lock::get_lock_status()
{
    return _lock_status;
}
