#include "pump.hpp"

#include "config.hpp"

#include <avr/io.h>
#include <util/delay.h>

#include <stdint.h>

namespace
{

    constexpr uint16_t kTimer1Top = 249; // ~500 Hz @ 8 MHz, prescaler 64

    uint16_t percent_to_duty(uint8_t percent)
    {
        if (percent > config::kMaxPercent)
        {
            percent = config::kMaxPercent;
        }

        uint16_t duty = static_cast<uint16_t>(
            (static_cast<uint32_t>(percent) * (kTimer1Top + 1)) / 100);

        if (duty > 0)
        {
            duty -= 1;
        }

        if (duty > kTimer1Top)
        {
            duty = kTimer1Top;
        }

        return duty;
    }

} // namespace

namespace pump
{

    void init()
    {
        // PB1 / OC1A as output
        DDRB |= (1 << DDB1);

        // Fast PWM, TOP = ICR1 (mode 14), non-inverting on OC1A
        TCCR1A = (1 << COM1A1) | (1 << WGM11);
        TCCR1B = (1 << WGM13) | (1 << WGM12);

        // Prescaler = 64
        TCCR1B |= (1 << CS11) | (1 << CS10);

        ICR1 = kTimer1Top;
        OCR1A = 0;
    }

    void set_percent(uint8_t percent)
    {
        OCR1A = percent_to_duty(percent);
    }

    void set_percent_with_boost(uint8_t percent)
    {
        if (percent == 0)
        {
            off();
            return;
        }

        set_percent(config::kPumpBoostPercent);
        _delay_ms(config::kPumpBoostMs);
        set_percent(percent);
    }

    void off()
    {
        set_percent(0);
    }

} // namespace pump