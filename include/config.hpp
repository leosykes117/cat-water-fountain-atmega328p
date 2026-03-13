#pragma once

#include <stdint.h>

namespace config
{

    // MCU / clock
    // constexpr uint32_t kCpuHz = 8000000UL;

    // Pump PWM behavior
    constexpr uint8_t kPumpTopPercent = 50;
    constexpr uint8_t kPumpBottomPercent = 8;
    constexpr uint8_t kPumpBoostPercent = 70;

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