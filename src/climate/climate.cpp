#include <climate.h>

#include <iostream>

Climate::getTemperature() const
{
    return temperature;
}

Climate::getHumidity() const
{
    return humidity;
}

Climate::getAirQualityIndex() const
{
    return airQualityIndex;
}

void Climate::setDPort(int dport)
{
    this->dport = dport;
}

int Climate::getDPort() const
{
    return dport;
}

void Climate::setAPort(int aport)
{
    this->aport = aport;
}

int Climate::getAPort() const
{
    return aport;
}

void Climate::readClimateData()
{
}
