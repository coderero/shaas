#include <services/config_engine.h>

/**
 * @brief Constructor for ConfigEngine.
 *        Allocates memory for the internal configuration data.
 */
ConfigEngine::ConfigEngine()
{
    _config = new config_data();
}

/**
 * @brief Destructor for ConfigEngine.
 *        Frees the allocated memory for configuration data.
 */
ConfigEngine::~ConfigEngine()
{
    delete _config;
}

/**
 * @brief Initializes the configuration engine.
 *        Begins EEPROM and loads configuration from EEPROM.
 *
 * @return true if configuration loaded or initialized successfully, false otherwise.
 */
bool ConfigEngine::init()
{
    EEPROM.begin();
    return load_config();
}

/**
 * @brief Loads configuration from EEPROM.
 *        If the version or size mismatch is detected, reinitializes default config.
 *
 * @return true if loaded successfully or reset correctly, false otherwise.
 */
bool ConfigEngine::load_config()
{
    EEPROM.get(EEPROM_ADDRESS, *_config);

    // Validate config version and size
    if (_config->version != 1 || _config->size != sizeof(config_data))
    {
        // Reinitialize default config structure
        _config->version = 1;
        _config->size = sizeof(config_data);
        _config->climate_size = 0;
        _config->ldr_size = 0;
        _config->motion_size = 0;
        _config->relay_size = 0;

        return save_config(); // Save initialized defaults
    }

    return true;
}

/**
 * @brief Saves the current configuration to EEPROM.
 *
 * @return true on successful write.
 */
bool ConfigEngine::save_config()
{
    EEPROM.put(EEPROM_ADDRESS, *_config);
    return true;
}

/**
 * @brief Sets the default configuration.
 *
 * @return true if set successfully, false otherwise.
 */
bool ConfigEngine::set_default_config()
{
    // Set default values for all configurations
    *_config = default_config;

    // Save the default configuration to EEPROM
    return save_config();
}

/**
 * @brief Sets the full configuration.
 *
 * @param config Configuration data to set.
 * @return true if successful, false otherwise.
 */
bool ConfigEngine::set_full_config(config_data config)
{
    // Validate the size of the new config
    if (config.size != sizeof(config_data))
        return false;

    // Copy the new config data
    *_config = config;

    // Save the new configuration to EEPROM
    return save_config();
}

/**
 * @brief Adds a new climate configuration.
 *
 * @param c Climate configuration to add.
 * @return true if added and saved successfully, false if storage is full.
 */
bool ConfigEngine::set_climate_config(climate c)
{
    if (_config->climate_size >= MAX_CLIMATE)
        return false;

    _config->climates[_config->climate_size++] = c;
    return save_config();
}

/**
 * @brief Adds a new LDR configuration.
 *
 * @param l LDR configuration to add.
 * @return true if added and saved successfully, false if storage is full.
 */
bool ConfigEngine::set_ldr_config(ldr l)
{
    if (_config->ldr_size >= MAX_LDR)
        return false;

    _config->ldrs[_config->ldr_size++] = l;
    return save_config();
}

/**
 * @brief Adds a new motion configuration.
 *
 * @param m Motion configuration to add.
 * @return true if added and saved successfully, false if storage is full.
 */
bool ConfigEngine::set_motion_config(motion m)
{
    if (_config->motion_size >= MAX_MOTION)
        return false;

    _config->motions[_config->motion_size++] = m;
    return save_config();
}

/**
 * @brief Retrieves a specific climate configuration by ID.
 *
 * @param id ID of the climate config.
 * @return The matched climate config or default-initialized if not found.
 */
climate ConfigEngine::get_climate_config(uint8_t id)
{
    for (int i = 0; i < _config->climate_size; ++i)
    {
        if (_config->climates[i].id == id)
            return _config->climates[i];
    }
    return {0};
}

/**
 * @brief Retrieves a specific LDR configuration by ID.
 *
 * @param id ID of the LDR config.
 * @return The matched LDR config or default-initialized if not found.
 */
ldr ConfigEngine::get_ldr_config(uint8_t id)
{
    for (int i = 0; i < _config->ldr_size; ++i)
    {
        if (_config->ldrs[i].id == id)
            return _config->ldrs[i];
    }
    return {0};
}

/**
 * @brief Retrieves a specific motion configuration by ID.
 *
 * @param id ID of the motion config.
 * @return The matched motion config or default-initialized if not found.
 */
motion ConfigEngine::get_motion_config(uint8_t id)
{
    for (int i = 0; i < _config->motion_size; ++i)
    {
        if (_config->motions[i].id == id)
            return _config->motions[i];
    }
    return {0};
}

/**
 * @brief Returns a pointer to the full configuration structure.
 *
 * @return Pointer to config_data object containing all settings.
 */
config_data *ConfigEngine::get_configs()
{
    return _config;
}
