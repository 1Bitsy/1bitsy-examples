/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
 * Copyright (C) 2011 Damjan Marion <damjan.marion@gmail.com>
 * Copyright (C) 2011 Mark Panajotovic <marko@electrontube.org>
 * Copyright (C) 2016 Piotr Esden-Tempski <piotr@esden.net>
 * Copyright (C) 2017 John Whitmore <johnfwhitmore@gmail.com>
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

#include "../common/button_boot.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>

/*
 * 168,000,000 MHz Clock so for 1 Second Interrupt need PRESCALE * PERIOD = 168M
 * Note the PRESCALE has to be at most a 16 Bit value.
 */
#define PRESCALE    10000
#define PERIOD      16800

/*
 * Timer_2 Interrupt Service Routine
 *
 * Simply check that we've got an Update Interrupt, Clear it and toggle the LED.
 * A fuller implementation would check that the ISR has not been called for some
 * other reason. For this example however I'll keep it simple.
 */
void tim2_isr(void)
{
        if(timer_interrupt_source(TIM2, TIM_SR_UIF)) {
	        timer_clear_flag(TIM2, TIM_SR_UIF);   // Clear the Interrup Flag
	        gpio_toggle(GPIOA, GPIO8);	      // Toggle LED.
	}
}

/* Set STM32 to 168 MHz. */
static void clock_setup(void)
{
	rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_168MHZ]);

	/* Enable GPIOA clock. */
	rcc_periph_clock_enable(RCC_GPIOA);
}

static void gpio_setup(void)
{
	/* Set GPIO8 (in GPIO port A) to 'output push-pull'. */
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT,
			GPIO_PUPD_NONE, GPIO8);
}

static void timer2_setup(void)
{
	rcc_periph_clock_enable(RCC_TIM2);
	rcc_periph_reset_pulse(RCC_APB1RSTR_TIM2RST);

	timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
	timer_set_prescaler(TIM2, PRESCALE);     // TIM2_PSC = PRESCALE;  
	timer_set_period(TIM2, PERIOD);          // TIM2_ARR = PERIOD;    Sets the auto-reload reg
	timer_enable_update_event(TIM2);         // TIM2_CR1.UDIS = 0;
	timer_update_on_any(TIM2);               // TIM2_CR1.URS = 0;
	timer_generate_event(TIM2, TIM_EGR_UG);  // TIM2_EGR.UG = 1;
	timer_enable_irq(TIM2, TIM_DIER_UIE);    // Enable the Interrupt
	timer_continuous_mode(TIM2);
	timer_enable_preload(TIM2);

	nvic_enable_irq(NVIC_TIM2_IRQ);          // Enable the Interrupt in the Interrupt Controller
	timer_enable_counter(TIM2);              // Start the counter.
}

int main(void)
{
	button_boot();

	clock_setup();
	gpio_setup();
	timer2_setup();
	
	/* Set two LEDs for wigwag effect when toggling. */
	gpio_set(GPIOA, GPIO8);

	/* Blink the LEDs (PA8) on the board. */
	while (1) {
	}

	return 0;
}
