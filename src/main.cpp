// #include <interfaces/IRFIDAuthenticator.h>

// #include <services/config_engine.h>
// #include <services/security.h>
// #include <sensors/climate.h>
// #include <sensors/pir.h>
// #include <sensors/ldr.h>
// #include <modules/mux.h>

// #define MUX_SIG 3
// #define MUX_S0 10
// #define MUX_S1 5
// #define MUX_S2 8
// #define MUX_S3 9

// class MockRFIDAuthenticator : public IRFIDAuthenticator
// {
// public:
//   bool is_authenticated(const byte *uid, byte length) override
//   {
//     Serial.print("MockRFIDAuthenticator: UID: ");
//     for (int i = 0; i < length; i++)
//     {
//       Serial.print(uid[i], HEX);
//       Serial.print(" ");
//     }
//     Serial.println();
//     return true; // Always return true for testing
//   }

//   bool register_uid(const byte *uid, byte length) override
//   {
//     Serial.print("MockRFIDAuthenticator: Registering UID: ");
//     for (int i = 0; i < length; i++)
//     {
//       Serial.print(uid[i], HEX);
//       Serial.print(" ");
//     }
//     Serial.println();
//     return true; // Always return true for testing
//   }

//   bool is_response_available() override
//   {
//     return true; // Always return true for testing
//   }
//   bool was_last_auth_successful() override
//   {
//     return true; // Always return true for testing
//   }

//   bool was_last_register_successful() override
//   {
//     return true; // Always return true for testing
//   }

//   ~MockRFIDAuthenticator() override
//   {
//     Serial.println("MockRFIDAuthenticator destroyed");
//   }
// };

// int selector_pins[] = {
//     MUX_S0,
//     MUX_S1,
//     MUX_S2,
//     MUX_S3,
// };

// Mux _mux;
// Security _security;

// ConfigEngine _config_engine;

// Climate _climates[MAX_CLIMATE];
// PIR _motion[MAX_MOTION];
// LDR _ldrs[MAX_LDR];

// config_data global_config = {
//     .version = 1,
//     .size = sizeof(config_data),
//     .climate_size = 1,
//     .ldr_size = 0,
//     .motion_size = 1,
//     .relay_size = 0,
//     .climates = {
//         {
//             .id = 1,
//             .dht22_port = 0,
//             .aqi_port = A0,
//             .has_buzzer = false,
//             .buzzer_port = 0,
//         },
//     },
//     .ldrs = {},
//     .motions = {
//         {
//             .id = 1,
//             .port = 1,
//             .relay_channel = 0,
//         },
//     },
//     .relays = {},
// };

// config_data *_config = &global_config;

// void setup()
// {
//   Serial.begin(9600);
//   while (!Serial)
//     ;

//   _mux.init(MUX_SIG, selector_pins, sizeof(selector_pins) / sizeof(selector_pins[0]),
//             DIGITAL, MUX_OUTPUT);
//   _security.init(new MockRFIDAuthenticator());

//   if (_config_engine.init())
//   {
//     Serial.println("Config Engine Initialized and Loaded Successfully!");
//   }
//   else
//   {
//     Serial.println("Failed to load config. Setting default values.");
//   }

//   if (_config)
//   {
//     Serial.println("Config Engine Loaded Successfully!");
//   }
//   else
//   {
//     Serial.println("Failed to load config.");
//   }

//   for (int i = 0; i < _config->climate_size; i++)
//   {
//     _climates[i].init(
//         _config->climates[i].id,
//         _config->climates[i].dht22_port,
//         _config->climates[i].aqi_port,
//         MUX_SIG,
//         &_mux);
//   }

//   for (int i = 0; i < _config->motion_size; i++)
//   {
//     _motion[i].init(_config->motions[i].id, &_mux);
//     _motion[i].set_port(_config->motions[i].port);
//   }
//
//   for (int i = 0; i < _config->ldr_size; i++)
//   {
//   _ldrs[i].init(_config->ldrs[i].id, _config->ldrs[i].port, _config->ldrs[i].threshold);
//   Serial.print("LDR ID: ");
//   Serial.println(_ldrs[i].get_id());
//   Serial.print("LDR Port: ");
//   Serial.println(_ldrs[i].get_port());
//   Serial.print("LDR Threshold: ");
//   Serial.println(_ldrs[i].get_threshold());
// }
// }

// void loop()
// {
//   if (_security.read_card())
//   {
//     Serial.println("Card read successfully.");
//   }
//   _security.handle();

//   if (_config->climate_size > 0)
//   {
//     for (int i = 0; i < _config->climate_size; i++)
//     {
//       _climates[i].read_climate_data();

//       Serial.print(_climates[i].get_temperature());
//       Serial.print(" ");
//       Serial.print(_climates[i].get_humidity());
//       Serial.print(" ");
//       Serial.print(_climates[i].get_air_quality_index());
//       Serial.println(" ");
//     }
//   }

//   if (_config->ldr_size > 0)
//   {
//     for (int i = 0; i < _config->ldr_size; i++)
//     {
//       _ldrs[i].read();
//       Serial.print("LDR Value: ");
//       Serial.println(_ldrs[i].read());
//     }
//   }

//   if (_config->motion_size > 0)
//   {
//     for (int i = 0; i < _config->motion_size; i++)
//     {
//       _motion[i].detect_movement();
//       Serial.print("Motion detected: ");
//       Serial.println(_motion[i].get_movement());
//     }
//   }
// }

#include <communication/mqtt_manager.h>

MQTTManager mqtt_manager;

void setup()
{
  mqtt_manager.begin("SmartHome-IOT", 10000);
}

void loop()
{
  mqtt_manager.loop();

  if (!mqtt_manager.is_connected())
  {
    // reset the wifi rom
    if (Serial.available())
    {
      char c = Serial.read();
      if (c == 'r')
      {
        for (int i = 0; i < CRED_EEPROM_SIZE; i++)
        {
          EEPROM.write(CRED_EEPROM_ADDR + i, 0);
        }

        Serial.println("EEPROM reset done. Please restart the device.");
      }
    }
    mqtt_manager.retry();
    return;
  }

  // Example: Publish a status message every 10 seconds
  static unsigned long last_publish = 0;
  if (millis() - last_publish >= 10000 && mqtt_manager.is_connected())
  {
    String status_topic = "device/SmartHome-IOT/status"; // Note: device_id not directly accessible
    mqtt_manager.publish(status_topic, "Device is online");
    last_publish = millis();
  }
}