/**
 * @file credentials.h
 * @brief Provides BLE-based configuration for WiFi and MQTT credentials with EEPROM persistence.
 *
 * This class allows receiving network credentials over BLE, parsing them from JSON,
 * and storing/loading them in EEPROM for persistent usage across reboots.
 */

#ifndef CREDENTIALS_H
#define CREDENTIALS_H

#pragma once

#include <Arduino.h>
#include <ArduinoBLE.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

/** @brief EEPROM start address for storing credentials. */
#define CRED_EEPROM_ADDR 1024

/** @brief EEPROM size allocated for credentials. */
#define CRED_EEPROM_SIZE 512

/** @brief Byte value used as a trigger to validate EEPROM data presence. */
#define CRED_TRIGGER_BYTE 0x42

/**
 * @struct network_config
 * @brief Holds the network configuration received over BLE.
 *
 * Contains the following fields:
 * - WiFi SSID and password
 * - MQTT broker address
 * - MQTT username and password
 */
typedef struct _c_t
{
    int device_id;        ///< Device ID
    String ssid;          ///< WiFi SSID
    String wifi_password; ///< WiFi password
    String mqtt_broker;   ///< MQTT broker address
    String mqtt_user;     ///< MQTT username
    String mqtt_password; ///< MQTT password
} network_config;

/**
 * @class Credentials
 * @brief Handles BLE-based credential input and EEPROM storage.
 */
class Credentials
{
private:
    BLEService _service;                  ///< BLE service for configuration
    BLECharacteristic _rx_characteristic; ///< BLE characteristic to receive JSON config

    const char *_ble_name;  ///< BLE device name
    bool _connected;        ///< BLE connection status
    bool _config_received;  ///< Flag to indicate if a valid config has been received
    bool _triggered;        ///< Internal flag to mark trigger byte verification
    network_config _config; ///< Parsed network configuration
    String _json_buffer;    ///< Buffer for partial JSON received over BLE

    // Static instance pointer for forwarding BLE events
    static Credentials *_instance;

    /**
     * @brief Called when a BLE central device connects.
     * @param central The connected BLE central device.
     */
    void _on_connect(BLEDevice central);

    /**
     * @brief Called when a BLE central device disconnects.
     * @param central The disconnected BLE central device.
     */
    void _on_disconnect(BLEDevice central);

    /**
     * @brief Handles incoming data from BLE and accumulates it into a JSON buffer.
     * @param data Pointer to received data.
     * @param length Length of the data received.
     */
    void _handle_bytes(const uint8_t *data, int length);

    /**
     * @brief Stores the JSON configuration string into EEPROM.
     * @param json The JSON string to be stored.
     */
    void _save_to_eeprom(const String &json);

    /**
     * @brief Loads JSON configuration from EEPROM into a string.
     * @param json_out Output string to store the loaded JSON data.
     * @return True if valid data was loaded, false otherwise.
     */
    bool _load_from_eeprom(String &json_out);

    /**
     * @brief Static handler for BLE characteristic writes.
     * @param central The BLE central device.
     * @param chr The BLE characteristic that was written.
     */
    static void _handle_bytes_static(BLEDevice central, BLECharacteristic chr);

    /**
     * @brief Static handler for BLE connection events.
     * @param central The BLE central device that connected.
     */
    static void _on_connect_static(BLEDevice central);

    /**
     * @brief Static handler for BLE disconnection events.
     * @param central The BLE central device that disconnected.
     */
    static void _on_disconnect_static(BLEDevice central);

public:
    /**
     * @brief Constructor for the Credentials class.
     */
    Credentials();

    /**
     * @brief Initializes the Credentials system with a BLE name.
     * @param ble_name The BLE name to advertise.
     */
    void init(const char *ble_name);

    /**
     * @brief Starts BLE services and prepares the module for operation.
     */
    void begin();

    /**
     * @brief Call this method regularly in the main loop to handle BLE events.
     */
    void poll();

    /**
     * @brief Checks if BLE is currently connected.
     * @return True if a central is connected, false otherwise.
     */
    bool is_connected() const;

    /**
     * @brief Checks if a new configuration has been received.
     * @return True if configuration is available, false otherwise.
     */
    bool config_available() const;

    /**
     * @brief Returns the parsed configuration structure.
     * @return A const reference to the `network_config` object.
     */
    const network_config &get_config() const;

    /**
     * @brief Attempts to load a previously saved configuration from EEPROM.
     * @return True if a valid configuration was loaded, false otherwise.
     */
    bool load_saved_config();

    /**
     * @brief Resets the stored configuration in EEPROM.
     */
    void reset_config();

    /**
     * @brief Cleans up the Credentials instance and stops BLE services.
     */
    void end();
};

#endif // CREDENTIALS_H