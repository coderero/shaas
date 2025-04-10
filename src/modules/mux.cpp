#include <modules/mux.h>

#include <Arduino.h>
#include <math.h>

/**
 * @brief Default constructor for Mux class
 * Leaves Mux uninitialized until init() is called
 */
Mux::Mux()
{
    signal_pin = -1;
    selection_pins = nullptr;
    selection_pins_size = 0;
    selected_channel = -1;
    mux_size = 0;
    initialized = false;
}

/**
 * @brief Initializes the multiplexer
 * Sets up pin modes and calculates channel range
 * @param signal_pin The pin used for signal input/output
 * @param selection_pins Array of pins used for channel selection
 * @param selection_pins_size Number of selection pins
 * @param mode Multiplexer mode (DIGITAL or ANALOG)
 * @param signal Signal direction (INPUT or OUTPUT)
 */
void Mux::init(int signal_pin, int *selection_pins, int selection_pins_size, mux_mode mode, signal_mode signal)
{
    this->signal_pin = signal_pin;
    this->selection_pins = selection_pins;
    this->selection_pins_size = selection_pins_size;
    this->mode = mode;
    this->signal = signal;

    this->mux_size = pow(2, selection_pins_size);

    // Set signal pin mode
    if (mode == DIGITAL && signal == MUX_OUTPUT)
    {
        pinMode(signal_pin, OUTPUT);
    }
    else
    {
        pinMode(signal_pin, INPUT);
    }

    // Set selection pins to output
    for (int i = 0; i < selection_pins_size; i++)
    {
        pinMode(selection_pins[i], OUTPUT);
    }

    initialized = true;
}

/**
 * @brief Destructor for Mux class
 * Frees memory for selection pins
 */
Mux::~Mux()
{
    free(selection_pins);
}

/**
 * @brief Selects a specific channel by setting selection pins
 * @param channel The channel index to select
 */
void Mux::channel(int channel)
{
    if (!initialized || channel < 0 || channel >= mux_size)
        return;

    selected_channel = channel;

    // Write binary value to selection pins
    for (int i = 0; i < selection_pins_size; i++)
    {
        int bit = (channel >> i) & 1;
        digitalWrite(selection_pins[i], bit);
    }
}

/**
 * @brief Sets the multiplexer operating mode
 * @param mux_mode DIGITAL or ANALOG
 */
void Mux::m_mode(mux_mode mux_mode)
{
    this->mode = mux_mode;
}

/**
 * @brief Sets the signal direction (input/output)
 * @param signal_mode MUX_INPUT or MUX_OUTPUT
 */
void Mux::s_mode(signal_mode signal_mode)
{
    this->signal = signal_mode;
}

/**
 * @brief Reads signal from the current channel
 * @return Digital HIGH/LOW signal if mode is DIGITAL and INPUT
 */
int Mux::read()
{
    if (!initialized || mode != DIGITAL || signal != MUX_INPUT)
        return 0;

    return digitalRead(signal_pin);
}

/**
 * @brief Writes a value to the current channel
 * @param value HIGH/LOW for digital, 0–255 for analog
 */
void Mux::write(int value)
{
    if (!initialized)
        return;

    if (mode == DIGITAL && signal == MUX_OUTPUT)
    {
        digitalWrite(signal_pin, value);
    }
    else
    {
        analogWrite(signal_pin, value);
    }
}

/**
 * @brief Gets the currently selected channel
 * @return Channel number (0 to mux_size - 1)
 */
int Mux::getSelectedChannel() const
{
    return selected_channel;
}

/**
 * @brief Gets the configured signal pin
 * @return Signal pin number
 */
int Mux::getSignalPin() const
{
    return signal_pin;
}
