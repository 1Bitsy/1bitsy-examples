/*
* This file is part of the libopencm3 project.
*
* Copyright (C) 2015-2016 Piotr Esden-Tempski <piotr@esden.net>
* Copyright (C) 2015 Jack Ziesing <jziesing@gmail.com>
*
* This library is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#include "../common/button_boot.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencmsis/core_cm3.h>

static void clock_setup(void)
{
    rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_168MHZ]);
}

static void gpiob6_setup(void)
{
    /* Enable GPIOB clock. */
    rcc_periph_clock_enable(RCC_GPIOB);
    /* Set GPIO6 (in GPIO port B) to 'output push-pull'. */
    gpio_mode_setup(GPIOB, GPIO_MODE_AF,
                    GPIO_PUPD_NONE, GPIO6);
    /* Set GPIO6 (in GPIO port B) alternate function */
    gpio_set_af(GPIOB, GPIO_AF2, GPIO6);
}

static void gpioa8_setup(void)
{
    /* Enable GPIOA clock. */
    rcc_periph_clock_enable(RCC_GPIOA);
    /* Set GPIO8 (in GPIO port A) to 'output push-pull'. */
    gpio_mode_setup(GPIOA, GPIO_MODE_AF,
                    GPIO_PUPD_NONE, GPIO8);
    /* Set GPIO8 (in GPIO port A) alternate function */
    gpio_set_af(GPIOA, GPIO_AF1, GPIO8);
}

static void tim4_setup(void)
{
    /* Enable TIM4 clock. */
    rcc_periph_clock_enable(RCC_TIM4);
    /* Reset TIM4 peripheral. */
    rcc_periph_reset_pulse(RCC_APB1RSTR_TIM4RST);
    /* Timer global mode:
    * - Sampling clock divider 1
    * - Alignment edge
    * - Direction up
    */
    timer_set_mode(TIM4, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
    /* Prescaler:
     * divide the input clock (half of the core clock aka 84MHz) by 128) */
    timer_set_prescaler(TIM4, 128);
    /* Enable preload. */
    timer_disable_preload(TIM4);
    /* Continous mode. */
    timer_continuous_mode(TIM4);
    /* Period (10Hz). (84MHz/128/65535) */
    timer_set_period(TIM4, 0xFFFF);
    /* Disable outputs. */
    timer_disable_oc_output(TIM4, TIM_OC1);
    timer_disable_oc_output(TIM4, TIM_OC2);
    timer_disable_oc_output(TIM4, TIM_OC3);
    timer_disable_oc_output(TIM4, TIM_OC4);
    /* -- OC1 configuration -- */
    /* Configure global mode of line 1. */
    timer_disable_oc_clear(TIM4, TIM_OC1);
    timer_enable_oc_preload(TIM4, TIM_OC1);
    timer_set_oc_slow_mode(TIM4, TIM_OC1);
    timer_set_oc_mode(TIM4, TIM_OC1, TIM_OCM_PWM1);
    timer_set_oc_polarity_high(TIM4, TIM_OC1);
    /* Set the capture compare value for OC1 to half of the period to achieve
     * 50% duty cycle.
     */
    timer_set_oc_value(TIM4, TIM_OC1, 0xFFFF/2);
    timer_enable_oc_output(TIM4, TIM_OC1);
    timer_enable_preload(TIM4);
    /* Counter enable. */
    timer_enable_counter(TIM4);
}

static void tim1_setup(void)
{
    /* Enable TIM1 clock. */
    rcc_periph_clock_enable(RCC_TIM1);
    /* Reset TIM1 peripheral. */
    rcc_periph_reset_pulse(RCC_APB1RSTR_TIM4RST);
    /* Timer global mode:
    * - Deadtime/sampling clock divider 1
    * - Alignment edge
    * - Direction up
    */
    timer_set_mode(TIM1, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
    /* Prescaler:
     * divide the input clock (full core clock aka 168MHz) by 128) */
    timer_set_prescaler(TIM1, 128);
    timer_set_repetition_counter(TIM1, 0);
    /* Enable preload. */
    timer_disable_preload(TIM1);
    /* Continous mode. */
    timer_continuous_mode(TIM1);
    /* Period (20Hz). (168MHz/128/65535) */
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
    /* Set the capture compare value for OC1 to half of the period to achieve
     * 50% duty cycle.
     */
    timer_set_oc_value(TIM1, TIM_OC1, 0xFFFF/2);
    timer_enable_oc_output(TIM1, TIM_OC1);
    timer_enable_preload(TIM1);
    timer_enable_break_main_output(TIM1);
    /* Counter enable. */
    timer_enable_counter(TIM1);
}

int main(void)
{
    button_boot();

    clock_setup();
    gpiob6_setup();
    tim4_setup();
    gpioa8_setup();
    tim1_setup();

    while (1) {
        __asm__("nop");
    }
    return 0;
}
