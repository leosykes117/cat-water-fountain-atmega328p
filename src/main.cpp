/**
 * cat-fountain — ATmega328P firmware
 *
 * Hardware:
 *   - MCU  : ATmega328P @ 8 MHz (external crystal)
 *   - Pump : DC via IRF520 MOSFET module, driven by Timer1 Fast-PWM on OC1A (PB1)
 *   - Level: Float switch on PD2 (INPUT_PULLUP — LOW = water present, HIGH = empty)
 *
 * Logic:
 *   - Timer1 Fast-PWM, TOP = ICR1, ~500 Hz
 *   - Float sensor with hysteresis: pump turns OFF below LOW_THRESHOLD,
 *     turns back ON only when water rises above HIGH_THRESHOLD
 *   - Soft-start (boost) every time the pump is enabled
 *   - Failsafe: if the sensor reads "empty" at startup, pump stays off
 */

#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <stdint.h>

// ---------------------------------------------------------------------------
// Pin / peripheral config
// ---------------------------------------------------------------------------

namespace pin
{
    // OC1A is fixed to PB1 by the hardware — Timer1 drives it directly
    constexpr uint8_t PUMP_DDR_BIT = DDB1;

    constexpr uint8_t FLOAT_PIN = PD2; // INT0 — using as digital input
    constexpr uint8_t FLOAT_DDR = DDD2;
}

// ---------------------------------------------------------------------------
// PWM — Timer1, Fast-PWM, TOP = ICR1
// ---------------------------------------------------------------------------

namespace pwm
{
    // ~500 Hz  →  8 000 000 / (64 × (249+1)) = 500 Hz
    constexpr uint16_t TOP = 249;
    constexpr uint8_t PRESCALER = (1 << CS11) | (1 << CS10); // /64

    void init()
    {
        DDRB |= (1 << pin::PUMP_DDR_BIT);

        // Fast-PWM, TOP = ICR1 (WGM=14), non-inverting on OC1A
        TCCR1A = (1 << COM1A1) | (1 << WGM11);
        TCCR1B = (1 << WGM13) | (1 << WGM12) | PRESCALER;

        ICR1 = TOP;
        OCR1A = 0;
    }

    // percent: 0–100
    void set_duty(uint8_t percent)
    {
        if (percent > 100)
            percent = 100;

        uint16_t duty = ((uint32_t)percent * (TOP + 1)) / 100;
        if (duty > 0)
            --duty;
        if (duty > TOP)
            duty = TOP;

        OCR1A = duty;
    }
}

// ---------------------------------------------------------------------------
// Pump — thin wrapper that owns the running state + soft-start
// ---------------------------------------------------------------------------

namespace pump
{
    constexpr uint8_t RUN_PERCENT = 40;   // normal operating duty (%)
    constexpr uint8_t BOOST_PERCENT = 60; // kick-start duty
    constexpr uint16_t BOOST_MS = 800;    // how long to boost

    static bool running = false;

    bool is_running() { return running; }

    void start()
    {
        if (running)
            return;
        running = true;

        // Boost so the impeller gets moving
        pwm::set_duty(BOOST_PERCENT);
        _delay_ms(BOOST_MS);
        pwm::set_duty(RUN_PERCENT);
    }

    void stop()
    {
        if (!running)
            return;
        running = false;
        pwm::set_duty(0);
    }
}

// ---------------------------------------------------------------------------
// Float sensor with hysteresis
//
//  Wiring: one leg to GND, other leg to PD2 (INPUT_PULLUP).
//          Float UP   → switch OPEN  → pin reads HIGH (1) → water present
//          Float DOWN → switch CLOSED→ pin reads LOW  (0) → tank empty
//
//  (If your module has inverted logic, just flip WATER_PRESENT below.)
// ---------------------------------------------------------------------------

namespace level
{
    // How many consecutive matching reads before we accept a state change.
    // At ~10 ms per iteration the default gives 200 ms debounce each direction.
    constexpr uint8_t DEBOUNCE_COUNTS = 20;

    // Hysteresis thresholds (in debounce counts — conceptual water "levels")
    // Here we model it simply: the sensor is digital, so hysteresis is achieved
    // by requiring the pump to stay off until the sensor confirms water is back
    // for a longer stretch than the OFF trigger.
    constexpr uint8_t COUNTS_TO_STOP = DEBOUNCE_COUNTS;      // reads "empty" before stopping
    constexpr uint8_t COUNTS_TO_START = DEBOUNCE_COUNTS * 3; // reads "full"  before restarting

    enum class State : uint8_t
    {
        UNKNOWN,
        FULL,
        EMPTY
    };

    static State state = State::UNKNOWN;
    static uint8_t empty_count = 0;
    static uint8_t full_count = 0;

    void init()
    {
        DDRD &= ~(1 << pin::FLOAT_DDR); // input
        // If we wanted to use the internal pull-up, we could do that here instead of wiring the sensor to GND:
        // PORTD |= (1 << pin::FLOAT_PIN); // enable pull-up
    }

    // Returns true when the pin reads "water present"
    static bool sensor_reads_water()
    {
        return (PIND & (1 << pin::FLOAT_PIN)) != 0; // HIGH = water present (see wiring note)
    }

    // Call periodically (~10 ms). Returns the debounced water state.
    State update()
    {
        if (sensor_reads_water())
        {
            ++full_count;
            empty_count = 0;
        }
        else
        {
            ++empty_count;
            full_count = 0;
        }

        // Clamp counters so they don't overflow
        if (full_count > COUNTS_TO_START)
            full_count = COUNTS_TO_START;
        if (empty_count > COUNTS_TO_STOP)
            empty_count = COUNTS_TO_STOP;

        // Hysteresis: only change state when the counter hits its threshold
        if (full_count >= COUNTS_TO_START)
            state = State::FULL;
        if (empty_count >= COUNTS_TO_STOP)
            state = State::EMPTY;

        return state;
    }

    State current() { return state; }
}

// ---------------------------------------------------------------------------
// Failsafe: watchdog reset if something goes wrong (WDT @ ~1 s)
// ---------------------------------------------------------------------------

namespace failsafe
{
    void init()
    {
        wdt_enable(WDTO_1S);
    }

    void kick()
    {
        wdt_reset();
    }
}

// ---------------------------------------------------------------------------
// Main control loop
// ---------------------------------------------------------------------------

int main()
{
    pwm::init();
    level::init();
    failsafe::init();

    // Seed the sensor state before touching the pump
    for (uint8_t i = 0; i < level::COUNTS_TO_START; ++i)
    {
        level::update();
        _delay_ms(10);
    }

    while (1)
    {
        failsafe::kick();

        const level::State water = level::update();

        if (water == level::State::EMPTY)
        {
            pump::stop();
        }
        else if (water == level::State::FULL)
        {
            pump::start();
        }
        // State::UNKNOWN → keep whatever we were doing last

        _delay_ms(10); // ~10 ms loop tick
    }
}