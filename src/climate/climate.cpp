#include <iostream>

#include <climate.h>
#include <mux.h>

#include <DHT.h>
#include <Air_Quality_Sensor.h>

Climate::Climate(int dport, const uint8_t &aport, int signal_pin, Mux *mux)
    : dport(dport), aport(aport), m(mux), dht(signal_pin, DHT22), air_quality_sensor(A0)
{
}

Climate::~Climate()
{
    // Destructor implementation (if needed)
}

bool Climate::init()
{
    // Initialize the DHT sensor
    dht.begin();

    // Initialize the air quality sensor
    if (!air_quality_sensor.init())
    {
        return false;
    }

    return true;
}

float Climate::getTemperature() const
{
    return temperature;
}

float Climate::getHumidity() const
{
    return humidity;
}

int Climate::getAirQualityIndex() const
{
    return airQualityIndex;
}

void Climate::setDPort(int d)
{
    dport = d;
}

int Climate::getDPort() const
{
    return dport;
}

void Climate::setAPort(int a)
{
    aport = a;
}

int Climate::getAPort() const
{
    return aport;
}

void Climate::readClimateData()
{
    // Select the appropriate mux channel for the DHT sensor
    m->channel(dport);
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();

    // Read air quality index from air quality sensor
    airQualityIndex = air_quality_sensor.getValue();

    // Check if any reads failed and handle error
    if (isnan(temperature) || isnan(humidity) || airQualityIndex < 0)
    {
        Serial.println("Failed to read from sensors!");
        return;
    }
}
