
// lcd.hpp
// Controls a 16x2 HD44780 LCD display through a PCF8574 I2C backpack.
// Provides initialization, text output, cursor positioning, and backlight control.
#pragma once

#include <stdint.h>

namespace lcd
{

    void init();
    void clear();
    void home();
    void set_cursor(uint8_t col, uint8_t row);
    void print(const char *str);
    void print_char(char c);
    void backlight_on();
    void backlight_off();
    void scroll_left();
    void scroll_right();
    void marquee(const char* str, uint8_t row, uint16_t step_delay_ms);

} // namespace lcd
