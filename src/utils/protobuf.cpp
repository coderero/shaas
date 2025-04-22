#include <utils/protobuf.h>
#include <services/whitelist_manager.h>
#include <pb_decode.h> // Include the header defining pb_istream_t
#include <transporter.pb.h>

bool encode_callback(pb_ostream_t *stream, const pb_field_t *field, void *const *arg)
{
    const uid_args *uid = static_cast<const uid_args *>(*arg);
    return pb_encode_tag_for_field(stream, field) &&
           pb_encode_string(stream, uid->uid, uid->length);
}

bool encode_string(pb_ostream_t *stream, const pb_field_t *field, void *const *arg)
{
    const char *str = (const char *)(*arg);

    if (!pb_encode_tag_for_field(stream, field))
        return false;

    return pb_encode_string(stream, (uint8_t *)str, strlen(str));
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

bool msg_callback(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    char *id_buffer = (char *)*arg;

    transporter_RegisterRequest req = transporter_RegisterRequest_init_zero;

    req.id.funcs.decode = decode_string;
    req.id.arg = id_buffer;

    if (!pb_decode(stream, transporter_RegisterRequest_fields, &req))
    {
        Serial.print("Failed to decode RegisterRequest: ");
        Serial.println(PB_GET_ERROR(stream));
        return false;
    }
    return true;
}