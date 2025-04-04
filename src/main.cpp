#define MUX_SIGNAL 3

#include <mux.h>
#include <climate.h>

const int selection_pins[] = {10, 5, 8, 9};

Mux mux(MUX_OUTPUT, (int *)selection_pins, sizeof(selection_pins) / sizeof(selection_pins[0]), DIGITAL, MUX_OUTPUT);
Climate climate(0, A0, MUX_SIGNAL, &mux);

void setup()
{
  Serial.begin(9600);
  Serial.println("Climate Sensor Initialization...");
  if (!climate.init())
  {
    Serial.println("Failed to initialize climate sensor!");
    while (1)
      ;
  }
  Serial.println("Climate Sensor Initialized Successfully!");
}

void loop()
{
  climate.readClimateData();

  Serial.print("Temperature: ");
  Serial.print(climate.getTemperature());
  Serial.println(" °C");

  Serial.print("Humidity: ");
  Serial.print(climate.getHumidity());
  Serial.println(" %");

  Serial.print("Air Quality Index: ");
  Serial.println(climate.getAirQualityIndex());

  delay(2000);
}