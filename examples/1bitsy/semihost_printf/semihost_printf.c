/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
 * Copyright (C) 2011 Stephen Caudle <scaudle@doceme.com>
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
#include <libopencm3/cm3/scs.h>

#include "../common/button_boot.h"
#include "uart.h"

extern void initialise_monitor_handles(void);

static void clock_setup(void)
{
	/* Enable LED GPIO. */
	rcc_periph_clock_enable(RCC_GPIOA);
}

static void gpio_setup(void)
{
	/* Setup GPIO pin GPIO8 on GPIO port A for LED. */
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO8);
}

int main(void)
{
	int i, j = 0;

	button_boot();

	clock_setup();
	gpio_setup();
	uart_init();

	/* Enable and initialise the debug monitor handler. */
    SCS_DEMCR |= SCS_DEMCR_VC_MON_EN;
	initialise_monitor_handles();

	/* Blink the LED (PA8) on the board with every transmitted byte sequence. */
	while (1) {
		gpio_toggle(GPIOA, GPIO8);	/* LED on/off */
		printf("0123456789");
		if ((j++ % 8) == 0) {		/* Newline after line full. */
			printf("\r\n");
		}
		fflush(0);
		for (i = 0; i < 100000; i++) {	/* Wait a bit. */
			__asm__("NOP");
		}
	}

	return 0;
}
