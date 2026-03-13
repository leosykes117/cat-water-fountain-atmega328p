#include "level_sensor.hpp"

#include "config.hpp"

#include <avr/io.h>
#include <util/delay.h>

#include <stdint.h>

namespace
{

    constexpr uint8_t kFloatPin = PD2;

    // External 10k pull-up to +5V on PD2
    // Float switch between PD2 and GND
    //
    // Logic:
    // LOW  => water OK
    // HIGH => low water OR sensor open/disconnected
    //
    // This is intentionally failsafe.
    bool read_raw_ok()
    {
        return (PIND & (1 << kFloatPin)) != 0;
    }

    bool confirm_state(bool expected_ok, uint16_t confirm_ms)
    {
        const uint16_t steps = confirm_ms / config::kSensorPollStepMs;

        for (uint16_t i = 0; i < steps; ++i)
        {
            _delay_ms(config::kSensorPollStepMs);

            if (read_raw_ok() != expected_ok)
            {
                return false;
            }
        }

        return true;
    }

} // namespace

namespace level_sensor
{

    void init()
    {
        DDRD &= ~(1 << DDD2); // input
        // PORTD &= ~(1 << PORTD2); // internal pull-up disabled
    }

    bool water_ok_raw()
    {
        return read_raw_ok();
    }

    bool water_low_raw()
    {
        return !read_raw_ok();
    }

    bool water_ok_confirmed()
    {
        if (!read_raw_ok())
        {
            return false;
        }

        return confirm_state(true, config::kWaterRecoveryConfirmMs);
    }

    bool water_low_confirmed()
    {
        if (read_raw_ok())
        {
            return false;
        }

        return confirm_state(false, config::kLowWaterConfirmMs);
    }

} // namespace level_sensor