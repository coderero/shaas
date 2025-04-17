#ifndef SECURITY_H
#define SECURITY_H

#include <modules/rfid.h>
#include <devices/lock.h>
#include <interfaces/IRFIDAuthenticator.h>

/**
 * @class Security
 * @brief Integrates RFID, lock, and authentication in a non-blocking secure system.
 */
class Security
{
private:
    RFID *rfid;                        ///< Pointer to RFID module
    Lock *lock;                        ///< Pointer to Lock module
    IRFIDAuthenticator *authenticator; ///< Interface to authentication logic

    bool _awaiting_auth_response = false;
    bool _awaiting_register_response = false;
    uint32_t _operation_start_time = 0;
    static constexpr uint32_t TIMEOUT = 5000; // Timeout for async responses

public:
    Security();
    ~Security();

    bool init(IRFIDAuthenticator *authenticator);

    /**
     * @brief Initiates card read and async authentication (non-blocking).
     * Call `handle()` to monitor result.
     */
    bool read_card();

    /**
     * @brief Starts async UID registration. Requires periodic `handle()` call.
     */
    void register_uid();

    /**
     * @brief Handles async state transitions for auth and lock operations.
     */
    void handle();
};

#endif // SECURITY_H
