#if !defined(CLIMATE_H)
#define CLIMATE_H

#include <string>

class Climate
{
private:
    int dport;
    int aport;
    float temperature;
    float humidity;
    int airQualityIndex;

public:
    Climate(int dport, int aport)
        : dport(dport), aport(aport), temperature(0.0f), humidity(0.0f), airQualityIndex(0) {};
    ~Climate();

    float getTemperature() const;

    float getHumidity() const;
    int getAirQualityIndex() const;

    void setDPort(int dport);
    int getDPort() const;

    void setAPort(int aport);
    int getAPort() const;

    void readClimateData();
}

#endif // CLIMATE_H
