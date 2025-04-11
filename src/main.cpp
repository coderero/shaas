#include <interfaces/IRFIDAuthenticator.h>
#include <services/security.h>

class MockAuthenticator : public IRFIDAuthenticator
{
public:
  bool is_authenticated(const byte *uid, byte length) override
  {
    // Convert UID to a string for easier comparison
    String uid_str = "";
    for (byte i = 0; i < length; i++)
    {
      uid_str += String(uid[i], HEX);
    }

    // Print the UID for debugging
    Serial.print("UID: ");
    Serial.println(uid_str);
    return true;
  }

  bool register_uid(const byte *uid, byte length) override
  {
    // Mock registration logic
    Serial.print("Registering UID: ");
    for (byte i = 0; i < length; i++)
    {
      Serial.print(uid[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
    return true;
  }
};

Security _security;

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting Security System...");

  MockAuthenticator *authenticator = new MockAuthenticator();
  if (_security.init(authenticator))
  {
    Serial.println("Security system initialized successfully.");
  }
  else
  {
    Serial.println("Failed to initialize security system.");
  }
}

void loop()
{
  if (_security.read_card())
  {
    Serial.println("Card read and authenticated.");
  }
}
