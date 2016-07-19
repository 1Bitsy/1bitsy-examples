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

#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

#include "adc.h"
#include "console.h"
#include "debounce.h"
#include "gpio.h"
#include "i2s.h"
#include "systick.h"
#include "warbler.h"

static const float F0 = 220.0;
static const float F1 = 1.2 * 220.0;

static warbler w1, w2;

static debounce trigger_button;

static const adc_channel slew_knob = { // PC0, ADC1 channel 10
    .adc_adc = ADC1,
    .adc_channel = 10,
    .adc_gpio = {
        .gp_port   = GPIOC,
        .gp_pin    = GPIO0,
        .gp_mode   = GPIO_MODE_ANALOG,
        .gp_pupd   = GPIO_PUPD_NONE,
        .gp_af     = 0,
        .gp_ospeed = GPIO_OSPEED_DEFAULT,
        .gp_otype  = GPIO_OTYPE_DEFAULT,
        .gp_level  = 0,
    },
};

static void setup_clocks(void)
{
    rcc_clock_setup_hse_3v3(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

    /* Enable TIM1 clock. */
    rcc_periph_clock_enable(RCC_TIM1);

    /* Enable GPIOA, Alternate Function clocks. */
    rcc_periph_clock_enable(RCC_GPIOA);

    /* Enable ADC1 clock. */
    rcc_periph_clock_enable(RCC_ADC1);

    /* Enable I2S clock. */
    RCC_CR |= RCC_CR_PLLI2SON;
}

static void setup_i2s(void)
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
    static const gpio_pin trigger_button_pin = {
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

    init_debounce(&trigger_button, &trigger_button_pin, BUTTON_SETTLE_MSEC);
}

static void setup_knobs(void)
{
    init_adc_channel(&slew_knob);
}

static void setup_warblers(void)
{
    init_warbler(&w1, F0, F1);
    init_warbler(&w2, 1.5 * F0, 1.5 * F1);
}

int main(void)
{
    setup_clocks();
    setup_systick(168000000);
    setup_console();
    setup_console_stdio();
    setup_LEDs();
    setup_button();
    setup_knobs();
    setup_warblers();
    setup_i2s();

    printf("Hello, World!\n");

    uint32_t next = 500;
    uint32_t next_adc = 1000;
    while (true) {

        // Red LED blinks.
        uint32_t now = system_millis;
        if (now == next) {
            gpio_toggle(GPIOB, GPIO0);
            next += 500;
        }

        // On-board LED lights when trigger button pressed.
        if (debounce_update(&trigger_button)) {
            if (debounce_is_falling_edge(&trigger_button)) {
                gpio_clear(GPIOA, GPIO8);
                printf("Click-"); fflush(stdout);
            }
            if (debounce_is_rising_edge(&trigger_button)) {
                gpio_set(GPIOA, GPIO8);
                printf("clack\n");
            }
        }

        // Slew knob value prints.
        if (now == next_adc) {
            printf("ADC = %d\n", adc_read_single(&slew_knob));
            next_adc += 200;
        }
    }

    return 0;
}