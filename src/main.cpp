// #include <interfaces/IRFIDAuthenticator.h>
// #include <services/security.h>

// class MockAuthenticator : public IRFIDAuthenticator
// {
// public:
//   bool is_authenticated(const byte *uid, byte length) override
//   {
//     // Convert UID to a string for easier comparison
//     String uid_str = "";
//     for (byte i = 0; i < length; i++)
//     {
//       uid_str += String(uid[i], HEX);
//     }

//     // Print the UID for debugging
//     Serial.print("UID: ");
//     Serial.println(uid_str);
//     return true;
//   }

//   bool register_uid(const byte *uid, byte length) override
//   {
//     // Mock registration logic
//     Serial.print("Registering UID: ");
//     for (byte i = 0; i < length; i++)
//     {
//       Serial.print(uid[i], HEX);
//       Serial.print(" ");
//     }
//     Serial.println();
//     return true;
//   }
// };

// Security _security;

// void setup()
// {
//   Serial.begin(115200);
//   Serial.println("Starting Security System...");

//   MockAuthenticator *authenticator = new MockAuthenticator();
//   if (_security.init(authenticator))
//   {
//     Serial.println("Security system initialized successfully.");
//   }
//   else
//   {
//     Serial.println("Failed to initialize security system.");
//   }
// }

// void loop()
// {
//   if (_security.read_card())
//   {
//     Serial.println("Card read and authenticated.");
//   }
// }

#include <Arduino.h>
#include <services/config_engine.h>

// Create an instance of ConfigEngine
ConfigEngine configEngine;

void setup()
{
  // Start serial communication
  Serial.begin(9600);
  while (!Serial)
    ; // Wait for Serial to be ready (optional)

  // Initialize the ConfigEngine
  if (configEngine.init())
  {
    Serial.println("Config Engine Initialized and Loaded Successfully!");
  }
  else
  {
    Serial.println("Failed to load config. Setting default values.");
  }

  // Set some sample climate configurations
  climate c1 = {1, 2, 3, true, 4};  // Sample climate with ID 1
  climate c2 = {2, 5, 6, false, 0}; // Sample climate with ID 2
  if (configEngine.set_climate_config(c1))
  {
    Serial.println("Climate 1 Config Saved Successfully.");
  }
  else
  {
    Serial.println("Failed to Save Climate 1 Config.");
  }

  if (configEngine.set_climate_config(c2))
  {
    Serial.println("Climate 2 Config Saved Successfully.");
  }
  else
  {
    Serial.println("Failed to Save Climate 2 Config.");
  }

  // Set some sample LDR configurations
  ldr l1 = {1, 7}; // Sample LDR with ID 1
  ldr l2 = {2, 8}; // Sample LDR with ID 2
  if (configEngine.set_ldr_config(l1))
  {
    Serial.println("LDR 1 Config Saved Successfully.");
  }
  else
  {
    Serial.println("Failed to Save LDR 1 Config.");
  }

  if (configEngine.set_ldr_config(l2))
  {
    Serial.println("LDR 2 Config Saved Successfully.");
  }
  else
  {
    Serial.println("Failed to Save LDR 2 Config.");
  }

  // Retrieve and display the configuration
  climate retrievedClimate = configEngine.get_climate_config(1);
  Serial.print("Retrieved Climate 1 ID: ");
  Serial.println(retrievedClimate.id);
  Serial.print("DHT22 Port: ");
  Serial.println(retrievedClimate.dht22_port);

  ldr retrievedLDR = configEngine.get_ldr_config(1);
  Serial.print("Retrieved LDR 1 ID: ");
  Serial.println(retrievedLDR.id);
  Serial.print("Port: ");
  Serial.println(retrievedLDR.port);
}

void loop()
{
  // Listen for serial input or perform other tasks
  if (Serial.available())
  {
    String input = Serial.readStringUntil('\n');
    Serial.print("Received: ");
    Serial.println(input);
    if (input == "reset")
    {
      // reset the configuration the eEPROM
      for (int i = 0; i < EEPROM_SIZE; i++)
      {
        EEPROM.write(i, 0XFF);
      }
    }
  }
}