#include "i2c.h"

#include <assert.h>
#include <stdio.h>

#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

#include "systick.h"

#define I2C_BAUD 100000
#define TIMEOUT_MSEC 25

// static void tim_setup(void)
// {
//     /* Enable TIM2 clock. */
//     rcc_periph_clock_enable(RCC_TIM2);

//     /* Reset TIM2 peripheral. */
//     timer_reset(TIM2);
//     /* Timer global mode:
//      * - No divider
//      * - Alignment edge
//      * - Direction up
//      */
//     timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
//     timer_set_prescaler(TIM2, 0);
//     timer_set_repetition_counter(TIM2, 0);
//     /* Enable preload. */
//     timer_disable_preload(TIM2);
//     /* Continous mode. */
//     timer_continuous_mode(TIM2);
//     /* Period (36kHz). */
//     timer_set_period(TIM2, 0xFFFFFFFF);
//     /* Disable outputs. */
//     timer_disable_oc_output(TIM2, TIM_OC1);
//     timer_disable_oc_output(TIM2, TIM_OC2);
//     timer_disable_oc_output(TIM2, TIM_OC3);
//     timer_disable_oc_output(TIM2, TIM_OC4);

//     timer_set_deadtime(TIM2, 0);
//     timer_set_enabled_off_state_in_idle_mode(TIM2);
//     timer_set_enabled_off_state_in_run_mode(TIM2);
//     timer_disable_break(TIM2);
//     timer_set_break_polarity_high(TIM2);
//     timer_disable_break_automatic_output(TIM2);
//     timer_set_break_lock(TIM2, TIM_BDTR_LOCK_OFF);

//     // /* -- OC1 configuration -- */
//     // /* Configure global mode of line 1. */
//     // timer_disable_oc_clear(TIM2, TIM_OC1);
//     // timer_enable_oc_preload(TIM2, TIM_OC1);
//     // timer_set_oc_slow_mode(TIM2, TIM_OC1);
//     // timer_set_oc_mode(TIM2, TIM_OC1, TIM_OCM_PWM1);
//     // timer_set_oc_polarity_high(TIM2, TIM_OC1);
//     // timer_set_oc_idle_state_set(TIM2, TIM_OC1);
//     // /* Set the capture compare value for OC1 to max value -1 for max
//     //    duty cycle/brightness. */
//     // timer_set_oc_value(TIM2, TIM_OC1, 0xFFFF/2);
//     // timer_enable_oc_output(TIM2, TIM_OC1);
//     // timer_enable_preload(TIM2);
//     // timer_enable_break_main_output(TIM2);
//     /* Counter enable. */
//     timer_enable_counter(TIM2);

//     uint32_t x[10];

//     for (int i = 0; i < 10; i++)
//         x[i] = timer_get_counter(TIM2);
//     for (int i = 0; i < 10; i++)
//         printf("timer = %ld\n", x[i]);
// }

// static void whatever(void)
// {
//     const uint32_t RCC_CFGR_SWS_MASK = 0x3;
//     const uint32_t RCC_PLLCFGR_PLLQ_MASK = 0xf;
//     const uint32_t RCC_PLLCFGR_PLLP_MASK = 0x3;
//     const uint32_t RCC_PLLCFGR_PLLN_MASK = 0x1ff;

//     uint32_t cfgr = RCC_CFGR;
//     uint32_t ppre2 = (cfgr >> RCC_CFGR_PPRE2_SHIFT) & RCC_CFGR_PPRE2_MASK;
//     uint32_t ppre1 = (cfgr >> RCC_CFGR_PPRE1_SHIFT) & RCC_CFGR_PPRE1_MASK;
//     uint32_t hpre = (cfgr >> RCC_CFGR_HPRE_SHIFT) & RCC_CFGR_HPRE_MASK;
//     uint32_t sws = (cfgr >> RCC_CFGR_SWS_SHIFT) & RCC_CFGR_SWS_MASK;

//     uint32_t pllcfgr = RCC_PLLCFGR;
//     uint32_t pllq = (pllcfgr >> RCC_PLLCFGR_PLLQ_SHIFT) & RCC_PLLCFGR_PLLQ_MASK;
//     uint32_t pllsrc = pllcfgr & RCC_PLLCFGR_PLLSRC;
//     uint32_t pllp = (pllcfgr >> RCC_PLLCFGR_PLLP_SHIFT) & RCC_PLLCFGR_PLLP_MASK;
//     uint32_t plln = (pllcfgr >> RCC_PLLCFGR_PLLN_SHIFT) & RCC_PLLCFGR_PLLN_MASK;
//     uint32_t pllm = (pllcfgr >> RCC_PLLCFGR_PLLM_SHIFT) & RCC_PLLCFGR_PLLM_MASK;

//     printf("CFGR:\n");
//     printf("    PPRE2  = %#lx\n", ppre2);
//     printf("    PPRE1  = %#lx\n", ppre1);
//     printf("    HPRE   = %#lx\n", hpre);
//     printf("    SWS    = %#lx\n", sws);
//     printf("\n");
//     printf("PLLCFGR:\n");
//     printf("    PLLQ   = %#lx\n", pllq);
//     printf("    PLLSRC = %#lx\n", pllsrc);
//     printf("    PLLP   = %#lx\n", pllp);
//     printf("    PLLN   = %#lx\n", plln);
//     printf("    PLLM   = %#lx\n", pllm);
//     printf("\n");
//     printf("AHB  = %ld\n", rcc_ahb_frequency);
//     printf("APB1 = %ld\n", rcc_apb1_frequency);
//     printf("APB2 = %ld\n", rcc_apb2_frequency);
//     printf("\n");
//     printf("\n");

//     // while (1)
//     //     ;
// }

void init_i2c(const i2c_config *ip)
{
    // Enable periph clock.
    // Configure GPIO.
    // Reset I²C.

    uint32_t base = ip->i_base_address;

    // tim_setup();
    // whatever();

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
    printf("i2c_freq_mhz = %lu\n", i2c_freq_mhz);
    printf("ccr = %lu\n", ccr);

    // I2C_CR1(base) = I2C_CR1_SWRST;
    // i2c_reset(base);
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
            // assert(system_millis < t0 + TIMEOUT_MSEC);
            if (system_millis >= t0 + TIMEOUT_MSEC) {
                fprintf(stderr, "Timeout on SB\n");
                printf("SR1 = %#lx\n", I2C_SR1(base));
                return;
            }
        }

        // Send slave address.
        I2C_DR(base) = cp->i_address & ~0x01;
        t0 = system_millis;
        while (!(I2C_SR1(base) & I2C_SR1_ADDR)) {
            // assert(!(I2C_SR1(base) & I2C_SR1_AF) && "i2c ack failed");
            // assert(system_millis < t0 + TIMEOUT_MSEC);
            if (I2C_SR1(base) & I2C_SR1_AF) {
                I2C_CR1(base) |= I2C_CR1_STOP;
                I2C_SR1(base) = ~I2C_SR1_AF;
                fprintf(stderr, "%ld: I2C ack failure on addr\n", system_millis / 1000);
                return;
            }
            if (system_millis >= t0 + TIMEOUT_MSEC) {
                fprintf(stderr, "Timeout on ADDR\n");
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
                // assert(system_millis < t0 + TIMEOUT_MSEC);
                if (system_millis >= t0 + TIMEOUT_MSEC) {
                    fprintf(stderr, "Timeout on BTF\n");
                    return;
                }
            }
        }
        I2C_CR1(base) |= I2C_CR1_STOP;
        printf("%lu: I2C transmit complete\n", system_millis / 1000);
    } else {
        assert(false && "slave transmission not implemented");
    }
}

void i2c_receive(const i2c_channel *cp, uint8_t *data, size_t count)
{
    cp = cp;                    // XXX
    data = data;                // XXX
    count = count;              // XXX
}
