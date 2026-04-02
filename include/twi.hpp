#ifndef __TWI_H__
#define __TWI_H__

#include <stdio.h>
#include <avr/io.h>

#if !defined(F_CPU)
#define F_CPU 8000000UL // Default to 8MHz if not defined
#endif                  // F_CPU

#define TWI_FREQ_HZ 100000UL // 100kHz
#define TWI_PRESCALER 1
#define TWI_STATUS_MASK 0xF8   // Mask for status bits in TWSR
#define TWI_START_ACK 0x08     // A START condition has been transmitted
#define TWI_REP_START_ACK 0x10 // A repeated START condition has been transmitted
#define TWI_SLAW_ACK 0x18      // SLA+W has been transmitted; ACK has been received
#define TWI_MT_DATA_ACK 0x28   // Data byte has been transmitted; ACK has been received

namespace twi
{
    /**
     * @desc    Set TWI pins (SDA and SCL) as inputs with pull-up resistors
     *
     * @param   void
     *
     * @return  void
     */
    void set_twi_pins(void);

    /**
     * @desc    TWI init - initialise communication
     *
     * @param   void
     *
     * @return  void
     */
    void init(void);

    /**
     * @desc    TWI start condition - send START signal to begin communication
     *
     * @param   void
     *
     * @return  void
     */
    void wait_till_twint_is_set(void);

    /**
     * @desc    TWI start condition - send START signal to begin communication
     */
    uint8_t start(void);

    /**
     * @desc    TWI stop condition - send STOP signal to end communication
     */
    void stop(void);

    /**
     * @desc   TWI check status - check TWSR status bits against expected value
     */
    bool check_status(uint8_t);

    /**
     * @desc    TWI enable ACK - enable acknowledgment for received data
     */
    void enable_ack(void);

    /**
     * @desc    TWI send device address with write bit to select client device
     *
     * @param  address - 7-bit I2C device address (without R/W bit)
     *
     * @return  uint8_t -
     */
    uint8_t send_address_w(uint8_t);

    uint8_t send_data(uint8_t);
}

#endif // __TWI_H__