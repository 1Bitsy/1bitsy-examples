#include "i2c.h"

#include <assert.h>
#include <stdio.h>

#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

#include "systick.h"

#define I2C_BAUD 100000
#define TIMEOUT_MSEC 25

void init_i2c(const i2c_config *ip)
{
    // Enable periph clock.
    // Configure GPIO.
    // Reset I²C.

    uint32_t base = ip->i_base_address;

    enum rcc_periph_clken clken;
    switch (base) {
    case I2C1:
        clken = RCC_I2C1;
        break;
    case I2C2:
        clken = RCC_I2C2;
        break;
    default:
        assert(false && "unknown I2C base address");
    }
    rcc_periph_clock_enable(clken);

    gpio_init_pins(ip->i_pins, (&ip->i_pins)[1] - ip->i_pins);

    uint32_t i2c_freq_mhz = rcc_apb1_frequency / 1000000;
    uint32_t ccr = rcc_apb1_frequency / I2C_BAUD / 2 + 1;
    if (ccr < 4)
        ccr = 4;

    i2c_peripheral_disable(base);
    i2c_set_clock_frequency(base, i2c_freq_mhz);
    i2c_set_trise(base, i2c_freq_mhz + 1);
    i2c_set_ccr(base, ccr);
    I2C_CR1(base) = 0;
    I2C_OAR1(base) = ip->i_own_address;
    i2c_peripheral_enable(base);
}

void i2c_transmit(const i2c_channel *cp, const uint8_t *data, size_t count)
{
    uint32_t base = cp->i_base_address;

    if (cp->i_is_master) {

        // Send start condition.
        I2C_CR1(base) |= I2C_CR1_START;
        uint32_t t0 = system_millis;
        while (!(I2C_SR1(base) & I2C_SR1_SB)) {
            if (system_millis >= t0 + TIMEOUT_MSEC) {
                fprintf(stderr, "i2c: timeout on SB\n");
                return;
            }
        }

        // Send slave address.
        I2C_DR(base) = cp->i_address & ~0x01;
        t0 = system_millis;
        while (!(I2C_SR1(base) & I2C_SR1_ADDR)) {
            if (I2C_SR1(base) & I2C_SR1_AF) {
                I2C_CR1(base) |= I2C_CR1_STOP;
                I2C_SR1(base) = ~I2C_SR1_AF;
                fprintf(stderr,
                        "i2c @ %ld: ack failure on addr\n",
                        system_millis / 1000);
                return;
            }
            if (system_millis >= t0 + TIMEOUT_MSEC) {
                fprintf(stderr, "i2c: timeout on ADDR\n");
                return;
            }
        }

        // Clear ADDR flag by reading SR1 and SR2 registers.
        uint16_t unused;
        unused = I2C_SR1(base);
        unused = I2C_SR2(base);
        unused = unused;

        // Write each data byte; wait for BTF.
        for (size_t i = 0; i < count; i++) {
            I2C_DR(base) = data[i];
            t0 = system_millis;
            while (!(I2C_SR1(base) & I2C_SR1_BTF)) {
                if (system_millis >= t0 + TIMEOUT_MSEC) {
                    fprintf(stderr, "i2c: timeout on BTF\n");
                    return;
                }
            }
        }
        I2C_CR1(base) |= I2C_CR1_STOP;
        fprintf(stderr, "i2c @ %lu: transmit complete\n", system_millis / 1000);
    } else {
        assert(false && "slave transmission not implemented");
    }
}

void i2c_receive(const i2c_channel *cp, uint8_t *data, size_t count)
{
    uint32_t base = cp->i_base_address;

    if (cp->i_is_master) {

        I2C_CR1(base) |= I2C_CR1_ACK;

        // Send start condition.
        I2C_CR1(base) |= I2C_CR1_START;
        uint32_t t0 = system_millis;
        while (!(I2C_SR1(base) & I2C_SR1_SB)) {
            if (system_millis >= t0 + TIMEOUT_MSEC) {
                fprintf(stderr, "i2c: timeout on SB\n");
                printf("SR1 = %#lx\n", I2C_SR1(base));
                return;
            }
        }

        // Send slave address.
        I2C_DR(base) = cp->i_address | 0x01;
        t0 = system_millis;
        while (!(I2C_SR1(base) & I2C_SR1_ADDR)) {
            if (I2C_SR1(base) & I2C_SR1_AF) {
                I2C_CR1(base) |= I2C_CR1_STOP;
                I2C_SR1(base) = ~I2C_SR1_AF;
                fprintf(stderr,
                        "i2c @ %ld: ack failure on addr\n",
                        system_millis / 1000);
                return;
            }
            if (system_millis >= t0 + TIMEOUT_MSEC) {
                fprintf(stderr, "i2c: timeout on ADDR\n");
                return;
            }
        }

        // Clear ADDR flag by reading SR1 and SR2 registers.
        uint16_t unused;
        unused = I2C_SR1(base);
        unused = I2C_SR2(base);
        unused = unused;

        // Read each data byte; wait for BTF.
        for (size_t i = 0; i < count; i++) {
            if (i + 1 == count)
                I2C_CR1(base) &= ~I2C_CR1_ACK;
            t0 = system_millis;
            while (true) {
                uint32_t sr1 = I2C_SR1(base);
                if (sr1 & (I2C_SR1_OVR | I2C_SR1_BERR)) {
                    fprintf(stderr, "i2c: receive error\n");
                    break;
                }
                if (sr1 & I2C_SR1_RxNE)
                    break;
                if (system_millis >= t0 + TIMEOUT_MSEC) {
                    fprintf(stderr, "i2c: timeout on receive\n");
                }
            }
            data[i] = I2C_DR(base);
            I2C_DR(base) = data[i];
        }
        printf("%lu: I2C receive complete\n", system_millis / 1000);
    } else {
        assert(false && "slave reception not implemented");
    }
}
