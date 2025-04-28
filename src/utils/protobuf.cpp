#include <utils/protobuf.h>
#include <services/whitelist_manager.h>
#include <pb_decode.h> // Include the header defining pb_istream_t
#include <transporter.pb.h>

bool encode_string(pb_ostream_t *stream, const pb_field_t *field, void *const *arg)
{
    const char *str = (const char *)(*arg);

    if (!pb_encode_tag_for_field(stream, field))
        return false;

    return pb_encode_string(stream, (uint8_t *)str, strlen(str));
}

bool encode_callback(pb_ostream_t *stream, const pb_field_t *field, void *const *arg)
{
    const uid_args *uid = static_cast<const uid_args *>(*arg);
    return pb_encode_tag_for_field(stream, field) &&
           pb_encode_string(stream, uid->uid, uid->length);
}

bool decode_string(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    char *dest_buffer = (char *)*arg;

    size_t len = stream->bytes_left;
    if (len > 127)
    {
        len = 127;
    }

    if (!pb_read(stream, (pb_byte_t *)dest_buffer, len))
    {
        Serial.println("Failed to read string data");
        return false;
    }

    dest_buffer[len] = '\0';
    return true;
}

bool decode_uid_bytes(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    // Get the shared data structure
    CallbackSharedData *shared_data = (CallbackSharedData *)*arg;

    // Calculate maximum buffer size
    size_t len = stream->bytes_left;
    if (len > MAX_UID_LENGTH)
    {
        len = MAX_UID_LENGTH;
    }

    // Read directly into our buffer
    if (!pb_read(stream, shared_data->uid_buffer, len))
    {
        Serial.println("Failed to read UID bytes");
        return false;
    }

    // Update the length
    shared_data->uid_length = len;
    shared_data->has_uid = (len > 0);

    // Debug output

    Serial.print("UID received: ");
    for (size_t i = 0; i < len; i++)
    {
        Serial.print(shared_data->uid_buffer[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    return true;
}

bool msg_callback(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    // Get the shared data structure
    CallbackSharedData *shared_data = (CallbackSharedData *)*arg;

    transporter_RegisterRequest req = transporter_RegisterRequest_init_zero;
    transporter_RevokeRequest revoke_req = transporter_RevokeRequest_init_zero;

    switch (field->tag)
    {
    case transporter_RfidEnvelope_register_request_tag:
        req.id.funcs.decode = decode_string;
        req.id.arg = shared_data->registration_id;

        if (!pb_decode(stream, transporter_RegisterRequest_fields, &req))
        {
            Serial.print("Failed to decode RegisterRequest: ");
            Serial.println(PB_GET_ERROR(stream));
            return false;
        }
        return true;

    case transporter_RfidEnvelope_register_response_tag:
        return true;

    case transporter_RfidEnvelope_revoke_request_tag:
        revoke_req.uid.value.funcs.decode = decode_uid_bytes;
        revoke_req.uid.value.arg = shared_data;

        if (!pb_decode(stream, transporter_RevokeRequest_fields, &revoke_req))
        {
            Serial.print("Failed to decode RevokeRequest: ");
            Serial.println(PB_GET_ERROR(stream));
            return false;
        }

        // Debug output moved to the decode_uid_bytes callback
        Serial.print("RevokeRequest received: ");
        for (size_t i = 0; i < shared_data->uid_length; i++)
        {
            Serial.print(shared_data->uid_buffer[i], HEX);
            Serial.print(" ");
        }
        Serial.println();

        return true;
    }

    return false;
}
bool config_callback(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    switch (field->tag)
    {
    case transporter_ConfigTopic_climate_tag:

        break;

    default:
        break;
    }
}