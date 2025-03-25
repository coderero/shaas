#include <sensors.pb.h>
#include <Arduino.h>
#include <pb_encode.h>

void prepare_sensor_data()
{
    SensorData sensor_data = SensorData_init_default;
    sensor_data.humidity = 50;
    sensor_data.temperature = 25;
    sensor_data.timestamp = 1234567890;

    uint8_t buffer[128];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    pb_encode(&stream, SensorData_fields, &sensor_data);

    Serial.write(buffer, stream.bytes_written);
}