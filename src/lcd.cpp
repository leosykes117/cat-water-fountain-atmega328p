
// lcd.cpp
// Implements HD44780 LCD control via PCF8574 I2C backpack in 4-bit mode.
#include "lcd.hpp"

#include "twi.hpp"

#include <avr/io.h>
#include <util/delay.h>

#include <stdint.h>

namespace
{

    // --- PCF8574 pin mapping (standard blue backpack) ---
    constexpr uint8_t kPinRS = 0x01;        // P0 = Register Select
    constexpr uint8_t kPinEN = 0x04;        // P2 = Enable
    constexpr uint8_t kPinBacklight = 0x08; // P3 = Backlight

    // --- HD44780 commands ---
    constexpr uint8_t kCmdClearDisplay = 0x01;
    constexpr uint8_t kCmdReturnHome = 0x02;
    constexpr uint8_t kCmdEntryModeSet = 0x04;
    constexpr uint8_t kCmdDisplayControl = 0x08;
    constexpr uint8_t kCmdFunctionSet = 0x20;
    constexpr uint8_t kCmdSetDdramAddr = 0x80;

    // --- Entry mode flags ---
    constexpr uint8_t kEntryLeft = 0x02;

    // --- Display control flags ---
    constexpr uint8_t kDisplayOn = 0x04;

    // --- Function set flags ---
    constexpr uint8_t kMode2Line = 0x08;

    // --- LCD dimensions ---
    constexpr uint8_t kCols = 16;
    constexpr uint8_t kRows = 2;

    // --- DDRAM row offsets ---
    constexpr uint8_t kRowOffset[] = {0x00, 0x40};

    // --- I2C address ---
    constexpr uint8_t kAddress = 0x27;

    // --- Mutable state ---
    uint8_t backlight_state = kPinBacklight;

    // Write a byte to the PCF8574 via I2C (start -> address -> data -> stop)
    uint8_t i2c_write(uint8_t data)
    {
        if (twi::start() != 0)
        {
            twi::stop();
            return 1;
        }
        if (twi::send_address_w(kAddress) != 0)
        {
            twi::stop();
            return 1;
        }
        if (twi::send_data(data) != 0)
        {
            twi::stop();
            return 1;
        }
        twi::stop();
        return 0;
    }

    // Pulse the Enable pin (high -> low) to latch data into HD44780
    void pulse_enable(uint8_t data)
    {
        i2c_write(data | kPinEN);
        _delay_us(1);
        i2c_write(data & ~kPinEN);
        _delay_us(50);
    }

    // Send a 4-bit nibble (data in upper 4 bits, mode = 0x00 for cmd or kPinRS for data)
    void send_nibble(uint8_t nibble, uint8_t mode)
    {
        uint8_t data = (nibble & 0xF0) | mode | backlight_state;
        pulse_enable(data);
    }

    // Send a full byte as two nibbles (high first, then low)
    void send_byte(uint8_t value, uint8_t mode)
    {
        send_nibble(value & 0xF0, mode);
        send_nibble(static_cast<uint8_t>(value << 4), mode);
    }

    // --- HD44780 display shift commands ---
    constexpr uint8_t kShiftDisplayLeft = 0x18;
    constexpr uint8_t kShiftDisplayRight = 0x1C;

    // Send a command (RS = 0)
    void send_command(uint8_t cmd)
    {
        send_byte(cmd, 0x00);
    }

    // Send character data (RS = 1)
    void send_char(uint8_t ch)
    {
        send_byte(ch, kPinRS);
    }

    // Variable delay in milliseconds (for runtime-determined durations)
    void delay_ms_var(uint16_t ms)
    {
        while (ms--)
        {
            _delay_ms(1);
        }
    }

    // Compute string length without pulling in <string.h>
    uint8_t str_len(const char* str)
    {
        uint8_t len = 0;
        while (str[len])
        {
            ++len;
        }
        return len;
    }

} // namespace

namespace lcd
{

    void init()
    {
        _delay_ms(50);

        i2c_write(backlight_state);
        _delay_ms(100);

        // Force 4-bit mode from any state (HD44780 datasheet Figure 24)
        send_nibble(0x30, 0x00);
        _delay_ms(5);
        send_nibble(0x30, 0x00);
        _delay_ms(5);
        send_nibble(0x30, 0x00);
        _delay_us(150);

        // Switch to 4-bit mode
        send_nibble(0x20, 0x00);
        _delay_us(150);

        // Function set: 4-bit, 2 lines, 5x8 font
        send_command(kCmdFunctionSet | kMode2Line);

        // Display on, cursor off, blink off
        send_command(kCmdDisplayControl | kDisplayOn);

        // Clear display
        send_command(kCmdClearDisplay);
        _delay_ms(2);

        // Entry mode: increment cursor, no display shift
        send_command(kCmdEntryModeSet | kEntryLeft);
    }

    void clear()
    {
        send_command(kCmdClearDisplay);
        _delay_ms(2);
    }

    void home()
    {
        send_command(kCmdReturnHome);
        _delay_ms(2);
    }

    void set_cursor(uint8_t col, uint8_t row)
    {
        if (row >= kRows)
        {
            row = kRows - 1;
        }
        if (col >= kCols)
        {
            col = kCols - 1;
        }
        send_command(kCmdSetDdramAddr | (kRowOffset[row] + col));
    }

    void print_char(char c)
    {
        send_char(static_cast<uint8_t>(c));
    }

    void print(const char *str)
    {
        while (*str)
        {
            print_char(*str);
            ++str;
        }
    }

    void backlight_on()
    {
        backlight_state = kPinBacklight;
        i2c_write(backlight_state);
    }

    void backlight_off()
    {
        backlight_state = 0x00;
        i2c_write(backlight_state);
    }

    void scroll_left()
    {
        send_command(kShiftDisplayLeft);
    }

    void scroll_right()
    {
        send_command(kShiftDisplayRight);
    }

    void marquee(const char* str, uint8_t row, uint16_t step_delay_ms)
    {
        uint8_t len = str_len(str);

        if (len <= kCols)
        {
            set_cursor(0, row);
            print(str);
            return;
        }

        for (uint8_t pos = 0; pos <= len - kCols; ++pos)
        {
            set_cursor(0, row);
            for (uint8_t i = 0; i < kCols; ++i)
            {
                print_char(str[pos + i]);
            }
            delay_ms_var(step_delay_ms);
        }
    }

} // namespace lcd
