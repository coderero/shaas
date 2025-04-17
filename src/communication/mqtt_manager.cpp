#include <communication/mqtt_manager.h>

// Initialize static instance pointer
MQTTManager *MQTTManager::_instance = nullptr;

MQTTManager::MQTTManager()
    : _wifi_manager(_credentials),
      _mqtt_client(_wifi_client), // Initialize with WiFiClient
      _mqtt_connected(false),
      _last_connect_attempt(0)
{
    _instance = this; // Set static instance for callback
}

void MQTTManager::begin(const char *device_name, uint32_t timeout_ms)
{
    _wifi_manager.begin(device_name, timeout_ms);

    if (_wifi_manager.is_connected())
    {
        _client_id = _credentials.get_config().device_id;
        _mqtt_client.setId(_client_id);
        _mqtt_client.setUsernamePassword(_credentials.get_config().mqtt_user, _credentials.get_config().mqtt_password);
        _mqtt_client.onMessage(MQTTManager::on_message_received);

        Serial.println("📡 Initializing MQTT...");
        connect_to_mqtt();

        // Subscribe to a default topic
        String command_topic = "device/" + _client_id + "/command";
        subscribe(command_topic);
    }
}

void MQTTManager::loop()
{
    _wifi_manager.loop();
    _mqtt_connected = _mqtt_client.connected();
    if (_mqtt_connected)
    {
        _mqtt_client.poll();
    }

    if (!_mqtt_connected && _wifi_manager.is_connected())
    {
        unsigned long now = millis();
        if (now - _last_connect_attempt >= _reconnect_interval_ms)
        {
            connect_to_mqtt();
            _last_connect_attempt = now;
        }
    }
}

bool MQTTManager::is_connected() const
{
    // Rely on _mqtt_connected to avoid const-correctness issues
    return _wifi_manager.is_connected() && _mqtt_connected;
}

void MQTTManager::publish(const String &topic, const String &message)
{
    if (!is_connected())
    {
        Serial.println("❌ Cannot publish: MQTT not connected");
        return;
    }

    _mqtt_client.beginMessage(topic);
    _mqtt_client.print(message);
    _mqtt_client.endMessage();

    Serial.print("📤 Published to ");
    Serial.print(topic);
    Serial.print(": ");
    Serial.println(message);
}

void MQTTManager::subscribe(const String &topic)
{
    if (!is_connected())
    {
        Serial.println("❌ Cannot subscribe: MQTT not connected");
        return;
    }

    _mqtt_client.subscribe(topic);
    Serial.print("📥 Subscribed to ");
    Serial.println(topic);
}

void MQTTManager::retry()
{
    _wifi_manager.retry();
}

void MQTTManager::connect_to_mqtt()
{
    if (!_wifi_manager.is_connected())
    {
        Serial.println("❌ WiFi not connected, cannot connect to MQTT");
        _mqtt_connected = false;
        return;
    }

    Serial.print("🔄 Connecting to MQTT broker: ");
    Serial.println(_credentials.get_config().mqtt_broker);

    if (_mqtt_client.connect(_credentials.get_config().mqtt_broker.c_str(), 1883))
    {
        _mqtt_connected = true;
        Serial.println("✅ Connected to MQTT broker");
    }
    else
    {
        _mqtt_connected = false;
        Serial.print("❌ Failed to connect to MQTT, reason: ");
        Serial.println(_mqtt_client.connectError());
    }
}

void MQTTManager::on_message_received(int message_size)
{
    if (_instance)
    {
        String topic = _instance->_mqtt_client.messageTopic();
        String message;

        while (_instance->_mqtt_client.available())
        {
            message += (char)_instance->_mqtt_client.read();
        }

        Serial.print("📩 Received message on ");
        Serial.print(topic);
        Serial.print(": ");
        Serial.println(message);

        // Add custom message handling logic here
    }
}