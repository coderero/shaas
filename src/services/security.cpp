#include <services/security.h>

Security::Security() : rfid(nullptr), lock(nullptr), authenticator(nullptr) {}

Security::~Security()
{
    delete rfid;
    delete lock;
}

bool Security::init(IRFIDAuthenticator *authenticator)
{
    if (!authenticator)
        return false;

    this->authenticator = authenticator;

    rfid = new RFID();
    if (!rfid->init())
        return false;

    lock = new Lock();
    return lock->init();
}

bool Security::read_card()
{
    if (!rfid || !authenticator || _awaiting_auth_response || _awaiting_register_response)
        return false; // Already processing

    if (rfid->read_card())
    {
        if (authenticator->is_authenticated(rfid->get_uid(), rfid->get_uid_length()))
        {
            _awaiting_auth_response = true;
            _operation_start_time = millis();
            return true;
        }
        else
        {
            // Optionally add feedback for failed authentication
        }
        {
            _awaiting_auth_response = true;
            _operation_start_time = millis();
            return true;
        }
    }
    return false;
}

void Security::register_uid()
{
    if (!rfid || !authenticator || _awaiting_register_response)
        return;

    if (rfid->read_card())
    {
        if (authenticator->register_uid(rfid->get_uid(), rfid->get_uid_length()))
        {
            _awaiting_register_response = true;
            _operation_start_time = millis();
        }
        else
        {
            // Optionally add feedback for failed registration
        }
    }
}

void Security::handle()
{
    if (lock)
        lock->handle();

    // Handle asynchronous authentication result
    if (_awaiting_auth_response)
    {
        if (authenticator->is_response_available() ||
            (millis() - _operation_start_time >= TIMEOUT))
        {
            _awaiting_auth_response = false;

            if (authenticator->was_last_auth_successful())
            {
                lock->unlock(); // Unlock on success
            }
            else
            {
                // Optionally add feedback for failed authentication
            }
        }
    }

    // Handle asynchronous registration result
    if (_awaiting_register_response)
    {
        if (authenticator->is_response_available() ||
            (millis() - _operation_start_time >= TIMEOUT))
        {
            _awaiting_register_response = false;

            if (authenticator->was_last_register_successful())
            {
                lock->lock(); // Lock only on successful registration
            }
            else
            {
                // Optionally log or flash error for failed registration
            }
        }
    }
}
