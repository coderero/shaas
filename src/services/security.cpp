#include <services/security.h>
#include <Arduino.h>

Security::Security(WhiteListManager *whitelist)
    : whitelist(whitelist), uid(nullptr)
{
}

Security::~Security()
{
    delete rfid;
    delete lock;
}

bool Security::init(WhiteListManager *whitelist)
{
    this->whitelist = whitelist;

    rfid = new RFID();
    lock = new Lock();

    if (!rfid || !lock)
    {
        Serial.println("Security: Failed to allocate RFID or Lock");
        return false;
    }

    if (!rfid->init())
    {
        Serial.println("Security: RFID initialization failed");
        return false;
    }

    if (!lock->init())
    {
        Serial.println("Security: Lock initialization failed");
        return false;
    }

    Serial.println("Security: Initialized successfully");
    return true;
}

void Security::enable_register_mode()
{
    if (_awaiting_auth_response || _awaiting_register_response)
    {
        Serial.println("Security: Operation in progress, cannot enable register mode");
        return;
    }
    _register_mode = true;
    Serial.println("Security: Entered registration mode");
}

void Security::handle()
{
    lock->handle();
    whitelist->update();

    if (whitelist->get_response())
    {
        if (whitelist->get_mode() == WhiteListMode::REGISTRATION)
        {
            _awaiting_register_response = false;
            _register_mode = false;
            Serial.println("Security: Registration completed");
        }
        else
        {
            _awaiting_auth_response = false;
            Serial.println("Security: Authentication completed");
        }
        whitelist->reset_response();
    }

    if (_awaiting_auth_response)
    {
        if (lock->get_lock_status() == LOCK)
        {
            _awaiting_auth_response = false;
        }
        else if (millis() - _operation_start_time > TIMEOUT)
        {
            _awaiting_auth_response = false;
            lock->lock();
        }
    }

    if (_awaiting_register_response)
    {
        if (millis() - _operation_start_time > TIMEOUT)
        {
            _awaiting_register_response = false;
            _register_mode = false;
            Serial.println("Security: Registration timeout, returning to auth mode");
        }
    }

    if (!_awaiting_auth_response && !_awaiting_register_response && rfid->read_card())
    {
        uint8_t *uid = rfid->get_uid();
        size_t len = rfid->get_uid_length();

        whitelist->set_uid(uid, len);

        if (whitelist->get_mode() == WhiteListMode::REGISTRATION)
        {
            _awaiting_register_response = true;
            _operation_start_time = millis();
        }
        else
        {
            if (whitelist->is_whitelisted(uid, len))
            {
                if (lock->unlock())
                {
                    _awaiting_auth_response = true;
                    _operation_start_time = millis();
                }
            }
            else
            {
                lock->lock();
            }
        }
    }
}
