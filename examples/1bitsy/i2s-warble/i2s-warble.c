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

#include <assert.h>
#include <stdio.h>

#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>

#include "adc.h"
#include "console.h"
#include "debounce.h"
#include "gpio.h"
#include "i2c.h"
#include "i2s.h"
#include "sgtl5000.h"
#include "systick.h"
#include "warbler.h"

static const float F0 = 220.0;
static const float F1 = 1.2 * 220.0;

static warbler w1, w2;
static volatile bool w1_empty;
static volatile int16_t w1_sample;
static volatile bool w2_empty;
static volatile int16_t w2_sample;

static debounce trigger_button;

#define CLOCK_SCALE (rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_168MHZ])

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

static int16_t next_sample(i2s_channel left_right)
{
    int16_t samp;

    switch (left_right) {

    case I2SC_LEFT:
        samp = w1_sample;
        w1_empty = true;
        break;

    case I2SC_RIGHT:
        samp = w2_sample;
        w2_empty = true;
        break;

    default:
        assert(false && "i2s callback: unknown I2S channel");
    }

    return samp;
}

static void setup_clocks(void)
{
    rcc_clock_setup_pll(&CLOCK_SCALE);
}

static void setup_i2s(void)
{
    static const i2s_config cfg = {
        .i2sc_sample_frequency = 44100,
        .i2sc_mode             = I2SM_MASTER_TX,
        .i2sc_standard         = I2SS_LSB,
        .i2sc_data_format      = I2SF_16,
        .i2sc_mclk_output      = I2SM_ENABLED,
        .i2sc_cpol             = I2SC_CPOL_HIGH,
        .i2sc_clock_source     = I2SC_PLL,
        .i2sc_full_duplex      = false,
        .i2sc_gpio_pins        = {
            {                   // LRCLK: Pin 21, PB12, AF5, I2S2_WS
                .gp_port       = GPIOB,
                .gp_pin        = GPIO12,
                .gp_mode       = GPIO_MODE_AF,
                .gp_pupd       = GPIO_PUPD_PULLUP,
                .gp_af         = GPIO_AF5,
                .gp_ospeed     = GPIO_OSPEED_2MHZ,
                .gp_otype      = GPIO_OTYPE_PP,
                .gp_level      = 0,
            },
            {                   // BCLK: Pin 22, PB13, AF5, I2S2_CK
                .gp_port       = GPIOB,
                .gp_pin        = GPIO13,
                .gp_mode       = GPIO_MODE_AF,
                .gp_pupd       = GPIO_PUPD_PULLUP,
                .gp_af         = GPIO_AF5,
                .gp_ospeed     = GPIO_OSPEED_2MHZ,
                .gp_otype      = GPIO_OTYPE_PP,
                .gp_level      = 0,
            },
            {                   // TX: Pin 24, PB15, AF5, I2S2_SD
                .gp_port       = GPIOB,
                .gp_pin        = GPIO15,
                .gp_mode       = GPIO_MODE_AF,
                .gp_pupd       = GPIO_PUPD_PULLUP,
                .gp_af         = GPIO_AF5,
                .gp_ospeed     = GPIO_OSPEED_2MHZ,
                .gp_otype      = GPIO_OTYPE_PP,
                .gp_level      = 0,
            },
            {                   // MCLK: Pin 25, PC6, AF5, I2S2_MCLK
                .gp_port       = GPIOC,
                .gp_pin        = GPIO6,
                .gp_mode       = GPIO_MODE_AF,
                .gp_pupd       = GPIO_PUPD_PULLUP,
                .gp_af         = GPIO_AF5,
                .gp_ospeed     = GPIO_OSPEED_25MHZ,
                .gp_otype      = GPIO_OTYPE_PP,
                .gp_level      = 0,
            },
        },
    };
    static const i2s_instance inst = {
        .i2si_base_address     = SPI2_BASE,
    };

    init_i2s(&cfg, &inst, next_sample);
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

static void setup_i2c(void)
{
    static const i2c_config sgtl_i2c = {
        .i_base_address    = I2C1,
        .i_own_address     = 32,
        .i_pins = {
            {                   // SCL: pin PB6, AF4
                .gp_port   = GPIOB,
                .gp_pin    = GPIO6,
                .gp_mode   = GPIO_MODE_AF,
                .gp_pupd   = GPIO_PUPD_NONE,
                .gp_af     = GPIO_AF4,
                .gp_ospeed = GPIO_OSPEED_50MHZ,
                .gp_otype  = GPIO_OTYPE_OD,
                .gp_level  = 1,
                
            },
            {                   // SDA: pin PB7, AF4
                .gp_port   = GPIOB,
                .gp_pin    = GPIO7,
                .gp_mode   = GPIO_MODE_AF,
                .gp_pupd   = GPIO_PUPD_NONE,
                .gp_af     = GPIO_AF4,
                .gp_ospeed = GPIO_OSPEED_50MHZ,
                .gp_otype  = GPIO_OTYPE_OD,
                .gp_level  = 1,
            },
        },
    };

    init_i2c(&sgtl_i2c);
}

static void setup_sgtl5000(void)
{
    static const i2c_channel sgtl = {
        .i_base_address = I2C1,
        .i_is_master = true,
        .i_stop = true,
        .i_address = 0b00010100,
    };
    init_sgtl5000(&sgtl);
    sgtl_set_volume(&sgtl, -6.0, -6.0);
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
    setup_i2c();
    setup_i2s();
    setup_sgtl5000();

    printf("Hello, World!\n");

    float slew = 0;
    uint32_t next_adc_time = 0;
    while (true) {
        uint32_t now = system_millis;
        if ((int32_t)(now - next_adc_time) >= 0) {
            slew = adc_read_single(&slew_knob) / 4095.0;
            next_adc_time += 10;
        }

        if (w1_empty) {
            w1_sample = warbler_next_sample(&w1, slew);
            w1_empty = false;
        }
        if (w2_empty) {
            w2_sample = warbler_next_sample(&w2, slew);
            w2_empty = false;
            if (!warbler_is_active(&w2))
                gpio_set(GPIOA, GPIO8);
        }

        if (debounce_update(&trigger_button)) {
            if (debounce_is_falling_edge(&trigger_button)) {
                gpio_clear(GPIOA, GPIO8);
                printf("Click-"); fflush(stdout);
                warbler_trigger(&w1, WS_RISING, slew);
                warbler_trigger(&w2, WS_FALLING, slew);
            }
            if (debounce_is_rising_edge(&trigger_button)) {
                // gpio_set(GPIOA, GPIO8);
                printf("clack\n");
            }
        }
    }

    return 0;
}
