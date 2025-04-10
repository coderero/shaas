#include <interfaces/IRFIDAuthenticator.h>
#include <modules/rfid.h>

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
};

RFID _rfid;

void setup()
{
  Serial.begin(9600);
  MockAuthenticator *authenticator = new MockAuthenticator();
  _rfid.init(authenticator);
}

void loop()
{
  // Simulate reading a card
  byte uid[] = {0xDE, 0xAD, 0xBE, 0xEF};

  if (_rfid.read_card())
  {
    Serial.println("Card read and authenticated.");
  }
}
