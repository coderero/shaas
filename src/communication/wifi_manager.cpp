#include <communication/wifi_manager.h>

WiFiManager::WiFiManager(Credentials &creds)
    : _credentials(creds), _connected(false), _timeout_ms(15000)
{
}

void WiFiManager::begin(const char *device_name, uint32_t timeout_ms)
{
    _timeout_ms = timeout_ms;

    Serial.begin(115200);
    while (!Serial)
        ;

    EEPROM.begin();
    _credentials.init(device_name);

    if (_credentials.load_saved_config())
    {
        Serial.println("✅ Loaded saved config from EEPROM");
        _cfg = _credentials.get_config();
        _is_configured = true;
        connect_to_wifi();
    }
    else
    {
        Serial.println("❌ Failed to load saved config from EEPROM");
        Serial.println("🔄 Starting BLE configuration...");
        _credentials.begin();
    }

    if (!_connected)
    {
        Serial.println("❌ Failed to connect to WiFi, starting fallback...");
        Serial.println("Please configure WiFi using the app and restart the device.");
        fallback_to_ble();
    }
}

void WiFiManager::raw_connect_to_wifi()
{
    if (WiFi.status() == WL_NO_MODULE)
    {
        Serial.println("❌ Device doesn't support WiFi");
        while (true)
            ;
    }
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < _timeout_ms)
    {
        WiFi.begin(_cfg.ssid.c_str(), _cfg.wifi_password.c_str());
        delay(1000);
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        _connected = true;
        Serial.println("\n✅ WiFi connected");
        Serial.print("📡 IP Address: ");
        Serial.println(WiFi.localIP());
    }
}

void WiFiManager::connect_to_wifi()
{
    if (WiFi.status() == WL_NO_MODULE)
    {
        Serial.println("❌ Device doesn't support WiFi");
        while (true)
            ;
    }
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < _timeout_ms)
    {
        WiFi.begin(_cfg.ssid.c_str(), _cfg.wifi_password.c_str());
        delay(1000);
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        _connected = true;
        Serial.println("\n✅ WiFi connected");
        Serial.print("📡 IP Address: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println("\n❌ Failed to connect to WiFi within timeout");
        _connected = false;
    }
}

void WiFiManager::fallback_to_ble()
{
    Serial.println("🔄 Starting BLE configuration...");
    _credentials.begin();
}

void WiFiManager::loop()
{
    _credentials.poll();

    if (_credentials.config_available())
    {
        if (!_is_configured)
        {
            _cfg = _credentials.get_config();
            _is_configured = true;
        }
    }
}

void WiFiManager::retry()
{
    if (_credentials.config_available())
    {
        _cfg = _credentials.get_config();
        _is_configured = true;

        Serial.println("🔄 Retrying WiFi connection... ");
        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < _timeout_ms)
        {
            WiFi.begin(_cfg.ssid.c_str(), _cfg.wifi_password.c_str());
            delay(1000);
        }
        if (WiFi.status() == WL_CONNECTED)
        {
            _connected = true;
        }
        else
        {
            Serial.println("\n❌ Failed to connect to WiFi within timeout");
        }
    }
}

bool WiFiManager::is_connected() const
{
    return _connected && WiFi.status() == WL_CONNECTED;
}

const network_config &WiFiManager::get_config() const
{
    return _cfg;
}
