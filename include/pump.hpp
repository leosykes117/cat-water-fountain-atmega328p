#pragma once

#include <stdint.h>

namespace pump
{

    void init();
    void set_percent(uint8_t percent);
    void set_percent_with_boost(uint8_t percent);
    void off();

} // namespace pump