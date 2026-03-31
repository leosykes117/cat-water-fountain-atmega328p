
// config.hpp
// Contains global configuration constants for the cat water fountain firmware.
// Defines pump PWM behavior, timing parameters, and sensor filtering settings.
// Centralizes all tunable values for easy adjustment and maintainability.
#pragma once

#include <stdint.h>

// Pump PWM behavior
#ifndef K_PUMP_TOP_PERCENT
#define K_PUMP_TOP_PERCENT 80
#endif

#ifndef K_PUMP_BOTTOM_PERCENT
#define K_PUMP_BOTTOM_PERCENT 8
#endif

#ifndef K_PUMP_BOOST_PERCENT
#define K_PUMP_BOOST_PERCENT 70
#endif

#ifndef K_MAX_PERCENT
#define K_MAX_PERCENT 100
#endif

namespace config
{

    // MCU / clock
    // constexpr uint32_t kCpuHz = 8000000UL;

    constexpr uint8_t kPumpTopPercent = K_PUMP_TOP_PERCENT;
    constexpr uint8_t kPumpBottomPercent = K_PUMP_BOTTOM_PERCENT;
    constexpr uint8_t kPumpBoostPercent = K_PUMP_BOOST_PERCENT;
    constexpr uint8_t kMaxPercent = K_MAX_PERCENT;

    constexpr uint16_t kPumpBoostMs = 800;
    constexpr uint16_t kPumpStartHoldMs = 1000;
    constexpr uint16_t kRampStepDelayMs = 150;
    constexpr uint16_t kRampEndHoldMs = 500;
    constexpr uint16_t kCyclePauseMs = 2000;

    // Level sensor filtering / hysteresis-like timing
    constexpr uint16_t kLowWaterConfirmMs = 500;
    constexpr uint16_t kWaterRecoveryConfirmMs = 800;
    constexpr uint16_t kSensorPollStepMs = 10;

    // Pin notes:
    // Pump PWM  -> OC1A / PB1 / Arduino D9
    // Float pin -> PD2  / Arduino D2 / physical pin 4

} // namespace config
