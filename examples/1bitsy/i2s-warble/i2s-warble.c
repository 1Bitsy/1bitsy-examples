/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2010 Piotr Esden-Tempski <piotr@esden.net>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>

#include "console.h"
#include "debounce.h"
#include "gpio.h"
#include "i2s.h"
#include "systick.h"

static debounce button;

static void setup_clocks(void)
{
    rcc_clock_setup_hse_3v3(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

    /* Enable TIM1 clock. */
    rcc_periph_clock_enable(RCC_TIM1);

    /* Enable GPIOA, Alternate Function clocks. */
    rcc_periph_clock_enable(RCC_GPIOA);

    /* Enable I2S clock. */
    RCC_CR |= RCC_CR_PLLI2SON;
}

#if 0
static void tim_setup(void)
{
	/* Reset TIM1 peripheral. */
	timer_reset(TIM1);
	/* Timer global mode:
	* - No divider
	* - Alignment edge
	* - Direction up
	*/
	timer_set_mode(TIM1, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
	timer_set_prescaler(TIM1, 0);
	timer_set_repetition_counter(TIM1, 0);
	/* Enable preload. */
	timer_disable_preload(TIM1);
	/* Continous mode. */
	timer_continuous_mode(TIM1);
	/* Period (36kHz). */
	timer_set_period(TIM1, 0xFFFF);
	/* Disable outputs. */
	timer_disable_oc_output(TIM1, TIM_OC1);
	timer_disable_oc_output(TIM1, TIM_OC2);
	timer_disable_oc_output(TIM1, TIM_OC3);
	timer_disable_oc_output(TIM1, TIM_OC4);

	timer_set_deadtime(TIM1, 0);
	timer_set_enabled_off_state_in_idle_mode(TIM1);
	timer_set_enabled_off_state_in_run_mode(TIM1);
	timer_disable_break(TIM1);
	timer_set_break_polarity_high(TIM1);
	timer_disable_break_automatic_output(TIM1);
	timer_set_break_lock(TIM1, TIM_BDTR_LOCK_OFF);

	/* -- OC1 configuration -- */
	/* Configure global mode of line 1. */
	timer_disable_oc_clear(TIM1, TIM_OC1);
	timer_enable_oc_preload(TIM1, TIM_OC1);
	timer_set_oc_slow_mode(TIM1, TIM_OC1);
	timer_set_oc_mode(TIM1, TIM_OC1, TIM_OCM_PWM1);
	timer_set_oc_polarity_high(TIM1, TIM_OC1);
	timer_set_oc_idle_state_set(TIM1, TIM_OC1);
	/* Set the capture compare value for OC1 to max value -1 for max duty cycle/brightness. */
	timer_set_oc_value(TIM1, TIM_OC1, 0xFFFF/2);
	timer_enable_oc_output(TIM1, TIM_OC1);
	timer_enable_preload(TIM1);
	timer_enable_break_main_output(TIM1);
	/* Counter enable. */
	timer_enable_counter(TIM1);

}
#endif

#if 0
static void i2s_setup(void)
{
    static const i2s_config cfg = {
        .i2sc_sample_frequency = 44100,
        .i2sc_mode             = I2SM_MASTER_TX,
        .i2sc_standard         = I2SS_LSB,
        .i2sc_data_format      = I2SF_16,
        .i2sc_mclk_output      = false,
        .i2sc_cpol             = I2SC_CPOL_HIGH,
        .i2sc_clock_source     = I2SC_PLL,
        .i2sc_full_duplex      = false,
    };
    static const i2s_instance inst = {
        .i2si_base_address     = SPI2_BASE,
    };

    init_i2s(&cfg, &inst);
}
#endif

static void setup_LEDs(void)
{
    static const gpio_pin LED_pins[] = {
        {                       // on-board LED: PA8.
            .gp_port   = GPIOA,
            .gp_pin    = GPIO8,
            .gp_mode   = GPIO_MODE_OUTPUT,
            .gp_pupd   = GPIO_PUPD_NONE,
            .gp_af     = GPIO_AF0,
            .gp_ospeed = GPIO_OSPEED_DEFAULT,
            .gp_otype  = GPIO_OTYPE_PP,
            .gp_level  = 1, // N.B., the on-board LED lights when signal is LOW.
        },
        {                       // red LED: PB0.
            .gp_port   = GPIOB,
            .gp_pin    = GPIO0,
            .gp_mode   = GPIO_MODE_OUTPUT,
            .gp_pupd   = GPIO_PUPD_NONE,
            .gp_af     = GPIO_AF0,
            .gp_ospeed = GPIO_OSPEED_DEFAULT,
            .gp_otype  = GPIO_OTYPE_PP,
            .gp_level  = 1,
        },
    };
    static const size_t LED_pin_count = (&LED_pins)[1] - LED_pins;
        
    gpio_init_pins(LED_pins, LED_pin_count);
}

static void setup_button(void)
{
    static const gpio_pin button_pin = {
        .gp_port   = GPIOB,
        .gp_pin    = GPIO8,
        .gp_mode   = GPIO_MODE_INPUT,
        .gp_pupd   = GPIO_PUPD_PULLUP,
        .gp_af     = GPIO_AF0,
        .gp_ospeed = GPIO_OSPEED_DEFAULT,
        .gp_otype  = GPIO_OTYPE_DEFAULT,
        .gp_level  = 0,
    };
    const uint32_t BUTTON_SETTLE_MSEC = 20;

    init_debounce(&button, &button_pin, BUTTON_SETTLE_MSEC);
}

int main(void)
{
	setup_clocks();
        setup_systick(168000000);
	// tim_setup();
        // i2s_setup();
        setup_console();
        setup_console_stdio();
        setup_LEDs();
        setup_button();

        printf("Hello, World!\n");

        uint32_t next = 500;
        while (true) {
            uint32_t now = system_millis;
            if (now == next) {
                gpio_toggle(GPIOB, GPIO0);
                next += 500;
            }
            if (debounce_update(&button)) {
                if (debounce_is_falling_edge(&button)) {
                    gpio_clear(GPIOA, GPIO8);
                    printf("Click-"); fflush(stdout);
                }
                if (debounce_is_rising_edge(&button)) {
                    gpio_set(GPIOA, GPIO8);
                    printf("clack\n");
                }
            }
        }

	return 0;
}
