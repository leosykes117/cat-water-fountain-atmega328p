
// app.cpp
// Implements the main application logic for the cat water fountain.
// Handles pump control cycles, water level checks, and system safety.
#include "app.hpp"

#include "config.hpp"
#include "level_sensor.hpp"
#include "pump.hpp"

#include <util/delay.h>

#include <stdint.h>

namespace
{

    void run_pump_cycle()
    {
        // Failsafe: si el agua no está confirmadamente OK, no arrancamos
        /* if (!level_sensor::water_ok_confirmed())
        {
            pump::off();
            return;
        } */

        pump::set_percent(config::kPumpTopPercent);
        _delay_ms(config::kPumpStartHoldMs);

        for (int8_t percent = config::kPumpTopPercent;
             percent >= config::kPumpBottomPercent;
             --percent)
        {
            // Si el nivel bajo se confirma, apagar inmediatamente
            if (level_sensor::water_low_confirmed())
            {
                pump::off();
                return;
            }

            pump::set_percent(static_cast<uint8_t>(percent));
            _delay_ms(config::kRampStepDelayMs);
        }

        _delay_ms(config::kRampEndHoldMs);
        pump::off();
        _delay_ms(config::kCyclePauseMs);
    }

} // namespace

namespace app
{

    void run()
    {
        pump::init();
        level_sensor::init();

        while (1)
        {
            // Failsafe: si el agua no está confirmadamente OK, no arrancamos
            if (!level_sensor::water_ok_confirmed())
            {
                pump::off();
                _delay_ms(200);
                continue;
            }
            pump::set_percent(config::kPumpTopPercent);
            //  run_pump_cycle();
        }
    }

} // namespace app