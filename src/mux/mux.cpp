#include <mux.h>
#include <math.h>

#include <Arduino.h>

/**
 * @brief Constructor for Mux class
 * @param signal_pin The pin used for signal input/output
 * @param selection_pins Array of pins used for channel selection
 * @param selection_pins_size Number of selection pins
 * @param mux_mode Multiplexer mode (DIGITAL or ANALOG)
 * @param signal Signal direction (INPUT or OUTPUT)
 */
Mux::Mux(int signal_pin, int *selection_pins, int selection_pins_size, mux_mode mode, signal_mode signal)
{
    this->signal_pin = signal_pin;
    this->selection_pins = selection_pins;
    this->selection_pins_size = selection_pins_size;
    this->mode = mode;
    this->signal = signal;

    // Calculate total number of channels based on selection pins (2^n)
    this->mux_size = pow(2, selection_pins_size);

    // Configure signal pin mode based on multiplexer mode and signal direction
    if (mode == DIGITAL && signal == OUTPUT)
    {
        pinMode(signal_pin, OUTPUT);
    }
    else
    {
        pinMode(signal_pin, INPUT);
    }

    // Set all selection pins as outputs
    for (int i = 0; i < selection_pins_size; i++)
    {
        pinMode(selection_pins[i], OUTPUT);
    }
}

/**
 * @brief Destructor for Mux class
 * Frees the memory allocated for selection pins
 */
Mux::~Mux()
{
    free(selection_pins);
}

/**
 * @brief Selects a specific channel on the multiplexer
 * @param channel The channel number to select (0 to mux_size-1)
 * @note Does nothing if channel is out of valid range
 */
void Mux::channel(int channel)
{
    if (channel < 0 || channel >= mux_size)
    {
        return;
    }
    selected_channel = channel;
}

/**
 * @brief Sets the multiplexer mode
 * @param mux_mode The mode to set (DIGITAL or ANALOG)
 */
void Mux::m_mode(mux_mode mux_mode)
{
    this->mode = mux_mode;
}

/**
 * @brief Sets the signal mode
 * @param signal_mode The signal direction to set (INPUT or OUTPUT)
 */
void Mux::s_mode(signal_mode signal_mode)
{
    this->signal = signal_mode;
}

/**
 * @brief Reads the current value from the selected channel
 * @return Digital value (HIGH/LOW) if in DIGITAL INPUT mode, undefined otherwise
 */
int Mux::read()
{
    if (mode == DIGITAL && signal == INPUT)
    {
        return digitalRead(signal_pin);
    }
    else
    {
        return 0;
    }
}

/**
 * @brief Writes a value to the selected channel
 * @param value The value to write (HIGH/LOW for digital, 0-255 for analog)
 * @note Uses digitalWrite for DIGITAL OUTPUT mode, analogWrite otherwise
 */
void Mux::write(int value)
{
    if (mode == DIGITAL && signal == OUTPUT)
    {
        digitalWrite(signal_pin, value);
    }
    else
    {
        analogWrite(signal_pin, value);
    }
}