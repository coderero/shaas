#include <WiFiS3.h>
#include <ArduinoMqttClient.h>

// WiFi credentials
const char *ssid = "";
const char *password = "";

// MQTT broker settings
const char *mqtt_server = "";
const int mqtt_port = 1883;

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char *topic = "arduino/sensor";
const long interval = 1000;
unsigned long previousMillis = 0;

long sensorValue = 0;

void setup()
{
  Serial.begin(9600);

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi");
  Serial.println("IP address: " + String(WiFi.localIP()));

  // Connect to MQTT broker
  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(mqtt_server);

  while (!mqttClient.connect(mqtt_server, mqtt_port))
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println("Connected to the MQTT broker!");
}

void loop()
{
  // Call poll() regularly to keep the MQTT connection alive
  mqttClient.poll();

  if (!mqttClient.connected())
  {
    Serial.println("Lost connection to the MQTT broker. Reconnecting...");
    mqttClient.connect(mqtt_server, mqtt_port);
    delay(1000);
  }

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;

    sensorValue += 1;

    // Publish message
    Serial.print("Sending message to topic: ");
    Serial.println(topic);

    mqttClient.beginMessage(topic);
    mqttClient.print("Sensor value: " + String(sensorValue));
    mqttClient.endMessage();
  }

  // Check WiFi connection and reconnect if needed
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi connection lost. Reconnecting...");
    WiFi.begin(ssid, password);
  }
}