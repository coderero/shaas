#if !defined(PROTOBUF_H)
#define PROTOBUF_H

#include <pb.h>
#include <pb_encode.h>

bool encode_callback(pb_ostream_t *stream, const pb_field_t *field, void *const *arg);

bool encode_string(pb_ostream_t *stream, const pb_field_t *field, void *const *arg);

bool decode_string(pb_istream_t *stream, const pb_field_t *field, void **arg);

bool msg_callback(pb_istream_t *stream, const pb_field_t *field, void **arg);

#endif // PROTOBUF_H
