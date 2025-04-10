#include <modules/rfid.h>

#include <SPI.h>

/**
 * @brief Constructor — does not perform hardware initialization.
 */
RFID::RFID()
    : rfid(nullptr),
      authenticator(nullptr)
{
}

/**
 * @brief Destructor — releases allocated MFRC522 instance.
 */
RFID::~RFID()
{
    if (rfid)
    {
        delete rfid;
        rfid = nullptr;
    }
}

/**
 * @brief Initializes the RFID reader with fixed SPI pins and sets the authenticator.
 *
 * Fixed pins:
 *  - Slave select (SS) is set to 4.
 *  - Reset (RST) is set to 2.
 *
 * SPI.begin() is called and a new MFRC522 instance is allocated accordingly.
 *
 * @param authenticator Pointer to an instance implementing IRFIDAuthenticator.
 * @return true if initialization was successful.
 */
bool RFID::init(IRFIDAuthenticator *authenticator)
{
    this->authenticator = authenticator;

    SPI.begin();
    // Use fixed pins: SS = 4, RST = 2
    rfid = new MFRC522(4, 2);
    rfid->PCD_Init();

    return true;
}

/**
 * @brief Reads a card and validates it through the authenticator.
 *
 * Checks for a new card and attempts to read its UID. If successful, it delegates
 * authentication to the provided IRFIDAuthenticator instance.
 *
 * @return true if a card was read and authenticated; false otherwise.
 */
bool RFID::read_card()
{
    if (!rfid || !authenticator)
    {
        return false;
    }

    if (!rfid->PICC_IsNewCardPresent() || !rfid->PICC_ReadCardSerial())
    {
        return false;
    }

    // Pass raw UID bytes to the authenticator
    bool is_authenticated = authenticator->is_authenticated(rfid->uid.uidByte, rfid->uid.size);

    rfid->PICC_HaltA();
    rfid->PCD_StopCrypto1();

    return is_authenticated;
}
