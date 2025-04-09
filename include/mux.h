#ifndef MUX_H
#define MUX_H

/**
 * @brief Enumeration for multiplexer operating modes
 */
typedef enum
{
    DIGITAL, ///< Digital mode for binary signals (HIGH/LOW)
    ANALOG   ///< Analog mode for continuous signals (PWM)
} mux_mode;

/**
 * @brief Enumeration for signal direction
 */
typedef enum
{
    MUX_INPUT, ///< Configure as input signal
    MUX_OUTPUT ///< Configure as output signal
} signal_mode;

/**
 * @brief Multiplexer control class
 * Provides an interface to control a CD74HC4067 multiplexer
 */
class Mux
{
private:
    int mux_size;            ///< Total number of available channels (2^n where n is selection_pins_size)
    int signal_pin;          ///< Pin used for signal input/output
    int *selection_pins;     ///< Array of pins used for channel selection
    int selection_pins_size; ///< Number of selection pins
    int selected_channel;    ///< Currently selected channel
    mux_mode mode;           ///< Current operating mode (DIGITAL/ANALOG)
    signal_mode signal;      ///< Current signal direction (INPUT/OUTPUT)
    bool initialized;        ///< Flag to indicate if Mux has been initialized

public:
    /**
     * @brief Default constructor for Mux class
     */
    Mux();

    /**
     * @brief Destructor for Mux class
     */
    ~Mux();

    /**
     * @brief Initializes the Mux with configuration
     * @param signal_pin The pin used for signal input/output
     * @param selection_pins Array of pins used for channel selection
     * @param selection_pins_size Number of selection pins
     * @param mode Multiplexer mode (DIGITAL or ANALOG)
     * @param signal Signal direction (INPUT or OUTPUT)
     */
    void init(int signal_pin, int *selection_pins, int selection_pins_size, mux_mode mode, signal_mode signal);

    /**
     * @brief Selects a specific channel on the multiplexer
     * @param channel The channel number to select (0 to mux_size - 1)
     */
    void channel(int channel);

    /**
     * @brief Sets the multiplexer operating mode
     * @param mux_mode The mode to set (DIGITAL or ANALOG)
     */
    void m_mode(mux_mode mux_mode);

    /**
     * @brief Sets the signal direction mode
     * @param signal_mode The signal direction to set (INPUT or OUTPUT)
     */
    void s_mode(signal_mode signal_mode);

    /**
     * @brief Reads the current value from the selected channel
     * @return Digital value (HIGH/LOW) if in DIGITAL INPUT mode, undefined otherwise
     */
    int read();

    /**
     * @brief Writes a value to the selected channel
     * @param value The value to write (HIGH/LOW for digital, 0-255 for analog)
     */
    void write(int value);

    /**
     * @brief Gets the currently selected channel
     * @return Selected channel index
     */
    int getSelectedChannel() const;

    /**
     * @brief Gets the signal pin used for the multiplexer
     * @return The signal pin number
     */
    int getSignalPin() const;
};

#endif
