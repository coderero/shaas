#include <pir.h>

#include <Arduino.h>

void PIR::set_port(int port)
{
    _port = port;
}

int PIR::get_port()
{
    return _port;
}

bool PIR::get_movement()
{
    return movement;
}

void PIR::detect_movement()
{
    if (digitalRead(_port) == HIGH)
    {
        movement = true;
    }
    else
    {
        movement = false;
    }
}
