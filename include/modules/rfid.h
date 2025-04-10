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
    MFRC522 *rfid;                     ///< Pointer to dynamically allocated MFRC522 instance
    IRFIDAuthenticator *authenticator; ///< Pointer to an authentication implementation

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
     * @brief Initializes the RFID reader and associates an authenticator.
     *
     * This method initializes SPI communication and allocates the MFRC522
     * instance using fixed pins (SS = 4, RST = 2).
     *
     * @param authenticator Pointer to an IRFIDAuthenticator that verifies tag validity.
     * @return true if the initialization was successful.
     */
    bool init(IRFIDAuthenticator *authenticator);

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
