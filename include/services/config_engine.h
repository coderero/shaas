/**
 * @file config_engine.h
 * @brief Configuration engine for managing dynamic IoT module settings using EEPROM.
 *
 * This module provides structured management of configuration for various IoT sensors and actuators,
 * including Climate sensors (DHT22, AQI, Buzzer), LDR sensors, Motion sensors, and Relays.
 * It uses EEPROM for persistent storage and supports versioning and structure validation.
 */

#ifndef CONFIG_ENGINE_H
#define CONFIG_ENGINE_H

#include <EEPROM.h>

// EEPROM configuration
#define EEPROM_SIZE 1024    ///< Total EEPROM size in bytes
#define EEPROM_ADDRESS 1024 ///< Starting address for config data in EEPROM

#define MAX_CLIMATE 2
#define MAX_LDR 2
#define MAX_MOTION 4

/**
 * @struct climate
 * @brief Configuration structure for climate modules.
 */
typedef struct _c
{
    uint8_t id;          ///< Unique identifier
    uint8_t dht22_port;  ///< DHT22 sensor port
    uint8_t aqi_port;    ///< AQI sensor port
    bool has_buzzer;     ///< Flag to indicate buzzer usage
    uint8_t buzzer_port; ///< Buzzer port (valid only if has_buzzer is true)
} climate;

/**
 * @struct ldr
 * @brief Configuration structure for LDR sensors.
 */
typedef struct _l
{
    uint8_t id;   ///< Unique identifier
    uint8_t port; ///< Analog pin used by LDR
} ldr;

/**
 * @struct motion
 * @brief Configuration structure for motion sensors (e.g., PIR).
 */
typedef struct _m
{
    uint8_t id;         ///< Unique identifier
    uint8_t port;       ///< PIR sensor pin
    uint8_t relay_port; ///< ID of the associated relay
    uint8_t relay_type; ///< Channel index of associated relay
} motion;

/**
 * @struct config_data
 * @brief Main configuration container for all sensor and relay settings.
 */
typedef struct _config_data
{
    uint8_t version;               ///< Configuration schema version
    uint8_t size;                  ///< Size of this structure
    uint8_t climate_size;          ///< Number of climate configs
    uint8_t ldr_size;              ///< Number of LDR configs
    uint8_t motion_size;           ///< Number of motion configs
    uint8_t relay_size;            ///< Number of relay configs
    climate climates[MAX_CLIMATE]; ///< Climate config array
    ldr ldrs[MAX_LDR];             ///< LDR config array
    motion motions[MAX_MOTION];    ///< Motion sensor config array
} config_data;

// Default configuration structure
const config_data default_config = {
    1,                   // version
    sizeof(config_data), // size
    0,                   // climate_size
    0,                   // ldr_size
    0,                   // motion_size
    0,                   // relay_size
    {},                  // climates
    {},                  // ldrs
    {},                  // motions
};

/**
 * @class ConfigEngine
 * @brief Class to manage sensor and relay configurations via EEPROM.
 */
class ConfigEngine
{
private:
    config_data *_config; ///< Internal pointer to config data

public:
    /**
     * @brief Constructs a new ConfigEngine object.
     */
    ConfigEngine();

    /**
     * @brief Destroys the ConfigEngine object.
     */
    ~ConfigEngine();

    /**
     * @brief Initializes the configuration system.
     *        Loads saved config or creates a default one if invalid.
     * @return true if successful, false if invalid config.
     */
    bool init();

    /**
     * @brief Loads configuration from EEPROM.
     * @return true if config is valid, false otherwise.
     */
    bool load_config();

    /**
     * @brief Saves the current configuration to EEPROM.
     * @return true if successful, false otherwise.
     */
    bool save_config();

    /**
     * @brief Sets the default configuration.
     * @return true if valid, false otherwise.
     */
    bool set_default_config();

    /**
     * @brief Sets the full configuration.
     * @param config Configuration data to set.
     * @return true if successful, false otherwise.
     * @note This function overwrites the existing configuration.
     */
    bool set_full_config(config_data config);

    /**
     * @brief Adds or updates a climate configuration.
     * @param climate Climate struct to add or update.
     * @return true if added or updated, false if list is full.
     */
    bool set_climate_config(climate climate);

    /**
     * @brief Adds or updates an LDR configuration.
     * @param ldr LDR struct to add or update.
     * @return true if added or updated, false if list is full.
     */
    bool set_ldr_config(ldr ldr);

    /**
     * @brief Adds or updates a motion sensor configuration.
     * @param motion Motion struct to add or update.
     * @return true if added or updated, false if list is full.
     */
    bool set_motion_config(motion motion);

    /**
     * @brief Retrieves a climate configuration by ID.
     * @param id Unique identifier of the climate config.
     * @return climate struct (returns default if not found).
     */
    climate get_climate_config(uint8_t id);

    /**
     * @brief Retrieves an LDR configuration by ID.
     * @param id Unique identifier of the LDR config.
     * @return ldr struct (returns default if not found).
     */
    ldr get_ldr_config(uint8_t id);

    /**
     * @brief Retrieves a motion configuration by ID.
     * @param id Unique identifier of the motion config.
     * @return motion struct (returns default if not found).
     */
    motion get_motion_config(uint8_t id);

    /**
     * @brief Deletes a climate configuration by ID.
     * @param id Unique identifier of the climate config to delete.
     */
    void delete_climate_config(uint8_t id);

    /**
     * @brief Deletes an LDR configuration by ID.
     * @param id Unique identifier of the LDR config to delete.
     */
    void delete_ldr_config(uint8_t id);

    /**
     * @brief Deletes a motion configuration by ID.
     * @param id Unique identifier of the motion config to delete.
     */
    void delete_motion_config(uint8_t id);

    /**
     * @brief Returns a pointer to the internal configuration data.
     * @return Pointer to config_data structure.
     */
    config_data *get_configs();
};

#endif // CONFIG_ENGINE_H
