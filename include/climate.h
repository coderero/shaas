#if !defined(CLIMATE_H)
#define CLIMATE_H

#include <string>

#include <mux.h>

#include <DHT.h>
#include <Air_Quality_Sensor.h>

class Climate
{
private:
    int dport;
    int aport;
    float temperature;
    float humidity;
    int airQualityIndex;

    Mux *m;
    DHT dht;
    AirQualitySensor air_quality_sensor;

public:
    Climate(int dport, const uint8_t &aport, int signal_pin, Mux *mux);
    ~Climate();

    bool init();

    float getTemperature() const;

    float getHumidity() const;
    int getAirQualityIndex() const;

    void setDPort(int dport);
    int getDPort() const;

    void setAPort(int aport);
    int getAPort() const;

    void readClimateData();
};

#endif