/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
 * Copyright (C) 2011 Stephen Caudle <scaudle@doceme.com>
 * Copyright (C) 2013 Piotr Esden-Tempski <piotr@esden.net>
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
#include <libopencm3/cm3/nvic.h>

uint32_t usart_table[6] = {
	USART1,
	USART2,
	USART3,
	UART4,
	UART5,
	USART6,
};

static void clock_setup(void)
{
	/* Enable clock for LED & USARTs. */
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_GPIOD);

	/* Enable clocks for USARTs. */
	rcc_periph_clock_enable(RCC_USART1);
	rcc_periph_clock_enable(RCC_USART2);
	rcc_periph_clock_enable(RCC_USART3);
	rcc_periph_clock_enable(RCC_UART4);
	rcc_periph_clock_enable(RCC_UART5);
	rcc_periph_clock_enable(RCC_USART6);
}

static void usart_config(uint32_t usart)
{
	/* Setup USART parameters. */
	usart_set_baudrate(usart, 38400);
	usart_set_databits(usart, 8);
	usart_set_stopbits(usart, USART_STOPBITS_1);
	usart_set_mode(usart, USART_MODE_TX_RX);
	usart_set_parity(usart, USART_PARITY_NONE);
	usart_set_flow_control(usart, USART_FLOWCONTROL_NONE);
}

static void usart_setup(void)
{
	/* Enable the USART interrupts. */
	nvic_enable_irq(NVIC_USART1_IRQ);
	nvic_enable_irq(NVIC_USART2_IRQ);
	nvic_enable_irq(NVIC_USART3_IRQ);
	nvic_enable_irq(NVIC_UART4_IRQ);
	nvic_enable_irq(NVIC_UART5_IRQ);
	nvic_enable_irq(NVIC_USART6_IRQ);

	/* Setup GPIO pins for USART transmit. */
	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6); /* USART1 */
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2); /* USART2 */
	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO10); /* USART3 */
	gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO10); /* UART4 */
	gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO12); /* UART5 */
	gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6); /* USART6 */

	/* Setup GPIO pins for USART receive. */
	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO7); /* USART1 */
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO3); /* USART2 */
	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO11); /* USART3 */
	gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO11); /* UART4 */
	gpio_mode_setup(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2); /* UART5 */
	gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO7); /* USART6 */

	/* Setup USART2 TX and RX pin as alternate function. */
	gpio_set_af(GPIOB, GPIO_AF7, GPIO6 | GPIO7); /* USART1 */
	gpio_set_af(GPIOA, GPIO_AF7, GPIO2 | GPIO3); /* USART2 */
	gpio_set_af(GPIOB, GPIO_AF7, GPIO10 | GPIO11); /* USART3 */
	gpio_set_af(GPIOC, GPIO_AF8, GPIO10 | GPIO11); /* UART4 */
	gpio_set_af(GPIOC, GPIO_AF8, GPIO12); /* UART5 */
	gpio_set_af(GPIOD, GPIO_AF8, GPIO2); /* UART5 */
	gpio_set_af(GPIOC, GPIO_AF8, GPIO6 | GPIO7); /* USART6 */

	/* Configure usart periph. */
	usart_config(USART1);
	usart_config(USART2);
	usart_config(USART3);
	usart_config(UART4);
	usart_config(UART5);
	usart_config(USART6);

	/* Enable USART2 Receive interrupt. */
	usart_enable_rx_interrupt(USART1);
	usart_enable_rx_interrupt(USART2);
	usart_enable_rx_interrupt(USART3);
	usart_enable_rx_interrupt(UART4);
	usart_enable_rx_interrupt(UART5);
	usart_enable_rx_interrupt(USART6);

	/* Finally enable the USARTs. */
	usart_enable(USART1);
	usart_enable(USART2);
	usart_enable(USART3);
	usart_enable(UART4);
	usart_enable(UART5);
	usart_enable(USART6);
}

static void gpio_setup(void)
{
	/* Setup GPIO pin GPIO8 on GPIO port A for LED. */
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO8);
}

int main(void)
{
	clock_setup();
	gpio_setup();
	usart_setup();

	while (1) {
		__asm__("NOP");
	}

	return 0;
}

static void usart_handler(int usart_id)
{
	static uint8_t data[6] = {'A','A','A','A','A','A'};
	uint32_t usart = usart_table[usart_id];

	/* Check if we were called because of RXNE. */
	if (((USART_CR1(usart) & USART_CR1_RXNEIE) != 0) &&
	    ((USART_SR(usart) & USART_SR_RXNE) != 0)) {

		/* Indicate that we got data. */
		gpio_toggle(GPIOA, GPIO8);

		/* Retrieve the data from the peripheral. */
		data[usart_id] = usart_recv(usart);

		/* Enable transmit interrupt so it sends back the data. */
		usart_enable_tx_interrupt(usart);
	}

	/* Check if we were called because of TXE. */
	if (((USART_CR1(usart) & USART_CR1_TXEIE) != 0) &&
	    ((USART_SR(usart) & USART_SR_TXE) != 0)) {

		/* Put data into the transmit register. */
		usart_send(usart, data[usart_id]);

		/* Disable the TXE interrupt as we don't need it anymore. */
		usart_disable_tx_interrupt(usart);
	}
}

void usart1_isr(void)
{
	usart_handler(0);
}

void usart2_isr(void)
{
	usart_handler(1);
}

void usart3_isr(void)
{
	usart_handler(2);
}

void uart4_isr(void)
{
	usart_handler(3);
}

void uart5_isr(void)
{
	usart_handler(4);
}

void usart6_isr(void)
{
	usart_handler(5);
}
