#ifndef RFID_H
#define RFID_H

#include <interfaces/IRFIDAuthenticator.h>

#include <MFRC522.h>

/**
 * @brief RFID module wrapper for the MFRC522 RFID reader.
 *
 * This class initializes the MFRC522 using pre-defined SPI pins (SS = 4, RST = 2)
 * and uses an external authenticator to validate RFID tag reads. The init() method
 * sets up the reader; hardware setup is not performed in the constructor.
 */
class RFID
{
private:
    MFRC522 *rfid; ///< Pointer to dynamically allocated MFRC522 instance

public:
    /**
     * @brief Default constructor (no hardware initialization)
     */
    RFID();

    /**
     * @brief Destructor cleans up allocated hardware resources
     */
    ~RFID();

    /**
     * @brief Returns the length of the UID of the last read card.
     *
     * @return Length of the UID in bytes.
     */
    byte *get_uid() { return rfid->uid.uidByte; } ///< Returns the UID of the last read card

    /**
     * @brief Returns the length of the UID of the last read card.
     *
     * @return Length of the UID in bytes.
     */
    byte get_uid_length() { return rfid->uid.size; } ///< Returns the length of the UID in bytes

    /**
     * @brief Initializes the RFID reader and associates an authenticator.
     *
     * This method initializes SPI communication and allocates the MFRC522
     * instance using fixed pins (SS = 4, RST = 2).
     *
     * @return true if the initialization was successful.
     */
    bool init();

    /**
     * @brief Attempts to read a card and authenticate it.
     *
     * The method checks for a new card, reads the UID, and then passes it to the
     * authenticator for validation. If a card is read and authenticated, it returns true.
     *
     * @return true if an authorized card was successfully read; false otherwise.
     */
    bool read_card();
};

#endif // RFID_H
