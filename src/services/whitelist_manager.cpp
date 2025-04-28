#include <services/whitelist_manager.h>
#include <utils/protobuf.h>

#include <pb_decode.h>
#include <pb_encode.h>
#include <transporter.pb.h>

void WhiteListManager::init(MQTTManager *mqtt_manager, const String &device_uid)
{
    this->device_uid = device_uid;
    mqtt = mqtt_manager;
}

void WhiteListManager::set_mode_registration()
{
    mode = WhiteListMode::REGISTRATION;
}

void WhiteListManager::set_uid(uint8_t *uid, size_t length)
{
    if (length > MAX_UID_LENGTH)
        return;
    memcpy(uid_buffer, uid, length);
    uid_length = length;
    new_uid_received = true;
}

void WhiteListManager::delete_uid(uint8_t *uid, size_t length)
{
    // Use the exact same comparison logic as is_whitelisted
    if (!uid || length == 0 || length > MAX_UID_LENGTH)
    {
        Serial.println("WhiteListManager: Invalid UID parameters for deletion");
        return;
    }

    Serial.print("Attempting to delete UID: ");
    for (size_t i = 0; i < length; i++)
    {
        Serial.print(uid[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    int index = W_EEPROM_ADDR;
    bool found = false;

    for (int i = 0; i < MAX_WHITELIST_SIZE; ++i)
    {
        uint8_t len = EEPROM.read(index);

        if (len == 0xFF || len == 0)
        {
            break;
        }

        // Only check if lengths match
        if (len == length)
        {
            bool match = true;

            // Compare each byte
            for (size_t j = 0; j < len; ++j)
            {
                uint8_t read_byte = EEPROM.read(index + 1 + j);
                if (read_byte != uid[j])
                {
                    match = false;
                    break;
                }
            }

            if (match)
            {
                Serial.print("Found matching UID at index ");
                Serial.println(index);

                // Mark this entry as deleted
                EEPROM.write(index, 0);

#if defined(ESP8266) || defined(ESP32)
                EEPROM.commit();
#endif

                Serial.println("UID deleted successfully");
                return;
            }
        }

        // Move to next entry
        index += 1 + len;
    }

    Serial.println("UID not found in whitelist, nothing to delete");
}
void WhiteListManager::update()
{
    if (!new_uid_received)
        return;

    switch (mode)
    {
    case WhiteListMode::AUTHENTICATION:
        if (is_whitelisted(uid_buffer, uid_length))
        {
            mqtt->publish("device/uid/authenticated", "true");
        }
        else
        {
            mqtt->publish("device/uid/authenticated", "false");
        }

        break;
    case WhiteListMode::REGISTRATION:
        awating_response = true;
        if (is_whitelisted(uid_buffer, uid_length))
        {
            mode = WhiteListMode::AUTHENTICATION;
            awating_response = false;
        }
        else
        {
            save_to_eeprom(uid_buffer, uid_length);
            publish_uid_for_registration();
            mode = WhiteListMode::AUTHENTICATION;
            awating_response = false;
        }
        break;
    }

    new_uid_received = false;
}

void WhiteListManager::save_to_eeprom(uint8_t *uid, size_t length)
{
    int index = W_EEPROM_ADDR;
    for (int i = 0; i < MAX_WHITELIST_SIZE; ++i)
    {
        uint8_t len = EEPROM.read(index);
        if (len == 0xFF || len == 0)
        {
            EEPROM.write(index, length);
            for (size_t j = 0; j < length; j++)
                EEPROM.write(index + 1 + j, uid[j]);
            return;
        }
        index += 1 + len;
    }
}

bool WhiteListManager::is_whitelisted(uint8_t *uid, size_t length)
{
    int index = W_EEPROM_ADDR;

    for (int i = 0; i < MAX_WHITELIST_SIZE; ++i)
    {
        uint8_t len = EEPROM.read(index);
        if (len == 0xFF || len == 0)
            break;

        bool match = (len == length);
        for (size_t j = 0; j < len; ++j)
        {
            uint8_t read_byte = EEPROM.read(index + 1 + j);
            if (j < length && read_byte != uid[j])
                match = false;
        }

        if (match)
        {
            return true;
        }

        index += 1 + len;
    }

    return false;
}

void WhiteListManager::publish_uid_for_registration()
{
    String topic = "arduino/" + device_uid + "/rfid";
    uint8_t buffer[256];

    transporter_UID uid = transporter_UID_init_zero;
    uid_args arg = {uid_buffer, uid_length};
    uid.value.funcs.encode = encode_callback;
    uid.value.arg = (void *)&arg;

    transporter_RegisterResponse response = transporter_RegisterResponse_init_zero;
    response.has_uid = true;
    response.uid = uid;

    response.id.funcs.encode = encode_string;
    response.id.arg = (void *)registration_request_id.c_str();

    transporter_RfidEnvelope rfidEnvelope = transporter_RfidEnvelope_init_zero;
    rfidEnvelope.which_payload = transporter_RfidEnvelope_register_response_tag;
    rfidEnvelope.payload.register_response = response;

    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    if (!pb_encode(&stream, transporter_RfidEnvelope_fields, &rfidEnvelope))
    {
        Serial.print("Failed to encode message: ");
        Serial.println(PB_GET_ERROR(&stream));
        return;
    }

    Serial.print("Message encoded successfully (");
    Serial.print(stream.bytes_written);
    Serial.println(" bytes)");

    mqtt->publish(topic.c_str(), buffer, stream.bytes_written);
}