/**
 * @file credentials.cpp
 * @brief Implements the Credentials class for BLE-based configuration with EEPROM persistence.
 */

#include <communication/credentials.h>

// Initialize the static instance pointer
Credentials *Credentials::_instance = nullptr;

/**
 * @brief Constructor for the Credentials class.
 * Initializes BLE service and characteristic with default UUIDs.
 */
Credentials::Credentials()
    : _service("1101"),
      _ble_name(nullptr),
      _rx_characteristic("1101", BLERead | BLEWrite, 512),
      _connected(false),
      _config_received(false),
      _triggered(false)
{
}

/**
 * @brief Initializes BLE with the provided name (local and advertised).
 * @param ble_name The name to use for the BLE device.
 */
void Credentials::init(const char *ble_name)
{
    _ble_name = ble_name;
}

/**
 * @brief Begins BLE advertising and sets up event handlers.
 */
void Credentials::begin()
{
    _instance = this;

    // Initialize BLE
    if (!BLE.begin())
    {
        Serial.println("❌ BLE initialization failed!");
        return;
    }

    // Set BLE device name and advertised service
    BLE.setLocalName(_ble_name);        // Set the local BLE name
    BLE.setDeviceName(_ble_name);       // Set the device name
    BLE.setAdvertisedService(_service); // Advertise the BLE service

    // Add the characteristic to the service
    _service.addCharacteristic(_rx_characteristic);

    // Add the service to the BLE stack
    BLE.addService(_service);

    // Start advertising the BLE service
    BLE.advertise();

    // Set up event handlers for BLE connection and disconnection
    BLE.setEventHandler(BLEConnected, Credentials::_on_connect_static);
    BLE.setEventHandler(BLEDisconnected, Credentials::_on_disconnect_static);

    // Set up the event handler for the characteristic write
    _rx_characteristic.setEventHandler(BLEWritten, Credentials::_handle_bytes_static);

    Serial.println("📡 BLE ready and advertising. Waiting for configuration...");
}
/**
 * @brief Poll BLE to handle events. Should be called regularly in loop().
 */
void Credentials::poll()
{
    BLE.poll();
}

/**
 * @brief Returns BLE connection status.
 * @return True if BLE central is connected, false otherwise.
 */
bool Credentials::is_connected() const
{
    return _connected;
}

/**
 * @brief Checks if a valid configuration has been received via BLE or EEPROM.
 * @return True if config is available, false otherwise.
 */
bool Credentials::config_available() const
{
    return _config_received;
}

/**
 * @brief Provides access to the parsed network configuration.
 * @return Const reference to the internal `network_config` struct.
 */
const network_config &Credentials::get_config() const
{
    return _config;
}

/**
 * @brief Internal handler when BLE central connects.
 * @param central BLE central device information.
 */
void Credentials::_on_connect(BLEDevice central)
{
    Serial.print("🔗 Connected: ");
    Serial.println(central.address());
    _connected = true;
    _triggered = false;
    _json_buffer = "";
}

/**
 * @brief Internal handler when BLE central disconnects.
 * @param central BLE central device information.
 */
void Credentials::_on_disconnect(BLEDevice central)
{
    Serial.print("🔌 Disconnected: ");
    Serial.println(central.address());
    _connected = false;
}

/**
 * @brief Handles and accumulates incoming bytes from BLE, parsing JSON when complete.
 * Directly appends incoming data to the JSON buffer and attempts to parse it as JSON.
 * @param data Byte array received from BLE.
 * @param length Length of the received data.
 */
void Credentials::_handle_bytes(const uint8_t *data, int length)
{
    // Append printable characters to the JSON buffer
    for (int i = 0; i < length; ++i)
    {
        char c = static_cast<char>(data[i]);
        if (isPrintable(c))
        {
            _json_buffer += c;
        }
    }

    // Attempt to parse JSON
    StaticJsonDocument<512> doc;
    if (deserializeJson(doc, _json_buffer) == DeserializationError::Ok)
    {
        // Successfully parsed JSON
        _config.device_id = doc["id"] | 0;
        _config.ssid = doc["ssid"] | "";
        _config.wifi_password = doc["password"] | "";
        _config.mqtt_broker = doc["broker"] | ""; // <-- fix this
        _config.mqtt_user = doc["user"] | "";     // <-- fix this
        _config.mqtt_password = doc["pass"] | ""; // <-- fix this
        _config_received = true;

        Serial.println("✅ Config received:");
        Serial.println(_json_buffer);

        // Save the JSON configuration to EEPROM
        _save_to_eeprom(_json_buffer);

        // Clear the buffer after successful parsing
        _json_buffer = "";
    }
}
/**
 * @brief Saves a JSON configuration string into EEPROM at predefined address.
 * @param json The JSON string to be saved.
 */
void Credentials::_save_to_eeprom(const String &json)
{
    int len = min(json.length(), CRED_EEPROM_SIZE - 1);
    for (int i = 0; i < len; ++i)
    {
        EEPROM.write(CRED_EEPROM_ADDR + i, json[i]);
    }

    EEPROM.write(CRED_EEPROM_ADDR + len, '\0'); // Null terminator

    Serial.println("💾 Config saved to EEPROM");
}

/**
 * @brief Loads configuration JSON from EEPROM into a string.
 * @param json_out Output reference to hold loaded string.
 * @return True if data is valid and loaded, false otherwise.
 */
bool Credentials::_load_from_eeprom(String &json_out)
{
    char buffer[CRED_EEPROM_SIZE];
    for (int i = 0; i < CRED_EEPROM_SIZE; ++i)
    {
        buffer[i] = EEPROM.read(CRED_EEPROM_ADDR + i);
        if (buffer[i] == '\0')
            break;
    }
    buffer[CRED_EEPROM_SIZE - 1] = '\0';
    json_out = String(buffer);

    return json_out.length() > 5; // Minimum length check for validity
}

/**
 * @brief Public wrapper to load and parse configuration from EEPROM.
 * @return True if config loaded and parsed successfully, false otherwise.
 */
bool Credentials::load_saved_config()
{
    String json;
    if (_load_from_eeprom(json))
    {
        StaticJsonDocument<512> doc;
        if (deserializeJson(doc, json) == DeserializationError::Ok)
        {
            _config.device_id = doc["id"] | 0;
            _config.ssid = doc["ssid"] | "";
            _config.wifi_password = doc["password"] | "";
            _config.mqtt_broker = doc["broker"] | ""; // <-- fix this
            _config.mqtt_user = doc["user"] | "";     // <-- fix this
            _config.mqtt_password = doc["pass"] | ""; // <-- fix this
            _config_received = true;

            Serial.println("📥 Loaded config from EEPROM:");
            Serial.println(json);
            return true;
        }
    }
    return false;
}

/**
 * @brief Resets the configuration stored in EEPROM to default values.
 * This function clears all bytes in the EEPROM area reserved for credentials.
 */
void Credentials::reset_config()
{
    for (int i = 0; i < CRED_EEPROM_SIZE; ++i)
    {
        EEPROM.write(CRED_EEPROM_ADDR + i, 0);
    }
    Serial.println("🔄 Config reset in EEPROM");
}

/**
 * @brief Static handler for BLE characteristic writes.
 * @param central The BLE central device.
 * @param chr The BLE characteristic that was written.
 */
void Credentials::_handle_bytes_static(BLEDevice central, BLECharacteristic chr)
{
    if (_instance)
    {
        _instance->_handle_bytes(chr.value(), chr.valueLength());
    }
}

/**
 * @brief Static handler for BLE connection events.
 * @param central The BLE central device that connected.
 */
void Credentials::_on_connect_static(BLEDevice central)
{
    if (_instance)
    {
        _instance->_on_connect(central);
    }
}

/**
 * @brief Static handler for BLE disconnection events.
 * @param central The BLE central device that disconnected.
 */
void Credentials::_on_disconnect_static(BLEDevice central)
{
    if (_instance)
    {
        _instance->_on_disconnect(central);
    }
}

/**
 * @brief Stops BLE advertising and disconnects the device.
 * This function should be called when the device is no longer in use.
 */
void Credentials::end()
{
    BLE.stopAdvertise();
    BLE.disconnect(); // optional
}