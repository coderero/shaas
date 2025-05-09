#if !defined(PROTOBUF_H)
#define PROTOBUF_H

#include <pb.h>
#include <pb_encode.h>

struct CallbackSharedData
{
    char registration_id[128]; // For register request
    uint8_t *uid_buffer;       // For revoke request UID
    size_t uid_length;         // Length of UID
    bool has_uid;              // Flag to indicate if we received a UID
};

struct WifiCredentials
{
    char ssid[64];
    size_t ssid_length;
    char password[64];
    size_t password_length;
};

bool encode_callback(pb_ostream_t *stream, const pb_field_t *field, void *const *arg);

bool encode_string(pb_ostream_t *stream, const pb_field_t *field, void *const *arg);

bool decode_string(pb_istream_t *stream, const pb_field_t *field, void **arg);

bool msg_callback(pb_istream_t *stream, const pb_field_t *field, void **arg);

bool config_callback(pb_istream_t *stream, const pb_field_t *field, void **arg);

#endif // PROTOBUF_H
