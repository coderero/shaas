#ifndef WHITELIST_MANAGER_H
#define WHITELIST_MANAGER_H

#define W_EEPROM_SIZE 1024
#define W_EEPROM_ADDR 512
#define MAX_UID_LENGTH 10
#define MAX_WHITELIST_SIZE 5

#include <functional>

#include <Arduino.h>
#include <EEPROM.h>
#include <communication/mqtt_manager.h>

enum class WhiteListMode
{
    AUTHENTICATION,
    REGISTRATION
};

struct uid_args
{
    uint8_t *uid;
    size_t length;
};

class WhiteListManager
{
    String device_uid, registration_request_id = "";
    MQTTManager *mqtt;
    WhiteListMode mode = WhiteListMode::AUTHENTICATION;
    uint8_t uid_buffer[MAX_UID_LENGTH];
    size_t uid_length = 0;
    bool new_uid_received = false;
    bool awating_response = false;

public:
    void init(MQTTManager *mqtt_manager, const String &device_uid);
    void update();
    void set_uid(uint8_t *uid, size_t length);
    bool is_whitelisted(uint8_t *uid, size_t length);
    void set_mode_registration();
    void reset_response() { awating_response = false; }
    void set_registration_request_id(const String &id) { registration_request_id = id; }

    bool get_response() const { return awating_response; }
    WhiteListMode get_mode() const { return mode; }

private:
    void save_to_eeprom(uint8_t *uid, size_t length);
    void publish_uid_for_registration();
};

#endif
