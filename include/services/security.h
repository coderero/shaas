#ifndef SECURITY_H
#define SECURITY_H

#include <modules/rfid.h>
#include <devices/lock.h>
#include <services/whitelist_manager.h>

/**
 * @class Security
 * @brief Integrates RFID, lock, and whitelist manager in a non-blocking secure system.
 */
class Security
{
private:
    byte *uid;                   ///< Array to store UID
    RFID *rfid;                  ///< Pointer to RFID module
    Lock *lock;                  ///< Pointer to Lock module
    WhiteListManager *whitelist; ///< Pointer to WhiteListManager for authentication/registration

    bool _awaiting_auth_response = false;
    bool _awaiting_register_response = false;
    bool _register_mode = false; ///< Flag for registration mode
    uint32_t _operation_start_time = 0;
    static constexpr uint32_t TIMEOUT = 5000; // Timeout for async responses

public:
    Security(WhiteListManager *whitelist);
    ~Security();

    bool init(WhiteListManager *whitelist);
    void enable_register_mode();
    void handle();
};

#endif // SECURITY_H
