#ifndef __TWI_CPP__
#define __TWI_CPP__

#include <stdio.h>
#include <avr/io.h>

#include "twi.hpp"

namespace
{
    /**
     * @desc    Convert prescaler value to TWPS bits for TWSR register
     * @param   p - prescaler value (1, 4, 16, 64)
     * @return  TWPS bits (0 for 1, 1 for 4, 2 for 16, 3 for 64)
     */
    constexpr uint8_t prescaler_to_twps(uint16_t p)
    {
        switch (p)
        {
        case 4:
            return 1;
        case 16:
            return 2;
        case 64:
            return 3;
        default:
            return 0; // prescaler = 1
        }
    }
}

namespace twi
{
    void set_twi_pins(void)
    {
        // Set SDA (PC4) and SCL (PC5) as inputs with pull-up resistors
        DDRC &= ~((1 << DDC4) | (1 << DDC5));   // Set as input
        PORTC |= (1 << PORTC4) | (1 << PORTC5); // Enable pull-up
    }

    void init(void)
    {
        set_twi_pins();

        // Set SCL frequency to 100kHz with 8MHz CPU clock
        // TWSR : Prescaler bits (1:0)
        TWSR = (TWSR & 0xFC) | prescaler_to_twps(TWI_PRESCALER);

        // TWBR : SCL = F_CPU / (16 + 2*TWBR*Prescaler)
        constexpr uint8_t twbr_value = static_cast<uint8_t>(
            ((F_CPU / TWI_FREQ_HZ) - 16U) / (2U * TWI_PRESCALER));

        TWBR = twbr_value;
        // Enable TWI
        TWCR = (1 << TWEN);
    }

    void wait_till_twint_is_set(void)
    {
        // Wait for TWINT flag to be set, indicating TWI operation has completed
        while (!(TWCR & (1 << TWINT)))
            ;
    }

    uint8_t start(void)
    {
        // Send START condition
        TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

        // this indicates START condition has been transmitted
        wait_till_twint_is_set();
        return (check_status(TWI_START_ACK) || check_status(TWI_REP_START_ACK)) ? 0 : 1;
    }

    void stop(void)
    {
        // Send STOP condition
        TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
    }

    bool check_status(uint8_t expected)
    {
        return (TWSR & TWI_STATUS_MASK) == expected;
    }

    void enable_ack(void)
    {
        // Enable ACK for received data
        TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
    }

    uint8_t send_address_w(uint8_t address)
    {
        // Load slave address into TWDR (shifted left by 1 for write mode)
        TWDR = (address << 1) | 0; // LSB = 0 for write

        // Clear TWINT to start transmission of address
        TWCR = (1 << TWINT) | (1 << TWEN);

        // Wait for address transmission to complete
        wait_till_twint_is_set();

        return check_status(TWI_SLAW_ACK) ? 0 : 1;
    }

    uint8_t send_data(uint8_t data)
    {
        // Load data byte into TWDR
        TWDR = data;

        // Clear TWINT to start transmission of data
        TWCR = (1 << TWINT) | (1 << TWEN);

        // Wait for data transmission to complete
        wait_till_twint_is_set();

        return check_status(TWI_MT_DATA_ACK) ? 0 : 1;
    }
}

#endif // __TWI_CPP__