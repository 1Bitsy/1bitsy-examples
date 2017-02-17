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

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>

static void clock_setup(void)
{
	/* Enable LED & USARTs. */
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);

	/* Enable clocks for USART2. */
	rcc_periph_clock_enable(RCC_USART1);
	rcc_periph_clock_enable(RCC_USART2);
	rcc_periph_clock_enable(RCC_USART3);
	rcc_periph_clock_enable(RCC_UART4);
	rcc_periph_clock_enable(RCC_UART5);
	rcc_periph_clock_enable(RCC_USART6);
}

static void usart_setup(uint32_t uart)
{
	/* Setup USART2 parameters. */
	usart_set_baudrate(uart, 38400);
	usart_set_databits(uart, 8);
	usart_set_stopbits(uart, USART_STOPBITS_1);
	usart_set_mode(uart, USART_MODE_TX);
	usart_set_parity(uart, USART_PARITY_NONE);
	usart_set_flow_control(uart, USART_FLOWCONTROL_NONE);

	/* Finally enable the USART. */
	usart_enable(uart);
}
	
static void usart_setup_all(void)
{
	usart_setup(USART1);
	usart_setup(USART2);
	usart_setup(USART3);
	usart_setup(UART4);
	usart_setup(UART5);
	usart_setup(USART6);
}

static void gpio_setup(void)
{
	/* Setup GPIO pin GPIO8 on GPIO port A for LED. */
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO8);
	
	/* Setup GPIO pins for USART1 transmit. */
	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6);

	/* Setup USART1 TX pin as alternate function. */
	gpio_set_af(GPIOB, GPIO_AF7, GPIO6);

	/* Setup GPIO pins for USART2 transmit. */
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2);

	/* Setup USART2 TX pin as alternate function. */
	gpio_set_af(GPIOA, GPIO_AF7, GPIO2);
	
	/* Setup GPIO pins for USART3 transmit. */
	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO10);

	/* Setup USART3 TX pin as alternate function. */
	gpio_set_af(GPIOB, GPIO_AF7, GPIO10);
	
	/* Setup GPIO pins for UART4 transmit. */
	gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO10);

	/* Setup UART4 TX pin as alternate function. */
	gpio_set_af(GPIOC, GPIO_AF8, GPIO10);
	
	/* Setup GPIO pins for UART5 transmit. */
	gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO12);

	/* Setup UART5 TX pin as alternate function. */
	gpio_set_af(GPIOC, GPIO_AF8, GPIO12);
	
	/* Setup GPIO pins for USART6 transmit. */
	gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6);

	/* Setup USART6 TX pin as alternate function. */
	gpio_set_af(GPIOC, GPIO_AF8, GPIO6);
}

static void usart_send_to_all_blocking(char c)
{
	usart_send_blocking(USART1, c);
	usart_send_blocking(USART2, c);
	usart_send_blocking(USART3, c);
	usart_send_blocking(UART4, c);
	usart_send_blocking(UART5, c);
	usart_send_blocking(USART6, c);
}

int main(void)
{
	int i, j = 0, c = 0;

	clock_setup();
	gpio_setup();
	usart_setup_all();

	/* Blink the LED (PA8) on the board with every transmitted byte. */
	while (1) {
		gpio_toggle(GPIOA, GPIO8);	/* LED on/off */
		usart_send_to_all_blocking(c + '0'); /* USART2: Send byte. */
		c = (c == 9) ? 0 : c + 1;	/* Increment c. */
		if ((j++ % 80) == 0) {		/* Newline after line full. */
			usart_send_to_all_blocking('\r');
			usart_send_to_all_blocking('\n');
		}
		for (i = 0; i < 100000; i++) {	/* Wait a bit. */
			__asm__("NOP");
		}
	}

	return 0;
}
