/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2014 Ken Sarkies <ksarkies@internode.on.net>
 * Copyright (C) 2017 Piotr Esden-Tempski <piotr@esden.net>
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
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/dac.h>
#include <libopencm3/stm32/dma.h>

#include <math.h>

/* Timer 2 count period, divides 84MHz APB2 clock */
#define PERIOD 293

/* PI * 2 is TAU */
#define M_TAU (M_PI * 2)

/* Globals */
uint8_t waveform[1024];
int wave_offset = 0;
float wave_frequency = 1.0;
float wave_direction = 1.0;
int wave_delay = 0;

/*--------------------------------------------------------------------*/
static int gen_sin_wave(uint8_t *buf, int len, int sample_offset, float samplerate, float frequency, float amplitude)
{
	gpio_set(GPIOA, GPIO1);

	for (int i = 0; i < len; i++) {
		buf[i] = ((sinf((M_TAU * frequency) * ((i + sample_offset) / samplerate)) * amplitude) + 1.0) * (0xFF / 2);
	}

	int ret = fmodf((len + sample_offset), (samplerate / frequency));

	gpio_clear(GPIOA, GPIO1);

	return ret;
}

/*--------------------------------------------------------------------*/

static int gen_square_wave(uint8_t *buf, int len, int sample_offset, float samplerate, float frequency, float amplitude)
{
	for (int i = 0; i < len; i++) {
		if (fmodf((i + sample_offset), (samplerate / frequency)) < (samplerate / frequency / 2)) {
			buf[i] = (0xFF / 2) + (0xFF * amplitude);
		} else {
			buf[i] = (0xFF / 2) - (0xFF * amplitude);
		}
	}

	return fmodf((len + sample_offset), (samplerate / frequency));
}

/*--------------------------------------------------------------------*/
static void clock_setup(void)
{
	rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_168MHZ]);
}

/*--------------------------------------------------------------------*/
static void gpio_setup(void)
{
	/* Port A and C are on AHB1 */
	rcc_periph_clock_enable(RCC_GPIOA);
	/* Set the digital test output on PA8 1Bitsy LED pin */
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO8);
	gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO8);
	/* Secondary debug output designating the sample generation time. */
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1);
	gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO1);
	/* Set PA4 for DAC channel 1 to analogue, ignoring drive mode. */
	/* Left channel on 1UP. */
	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO4);
	/* Set PA5 for DAC channel 2 to analogue, ignoring drive mode. */
	/* Right channel on 1UP. */
	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO5);
}

/*--------------------------------------------------------------------*/
static void timer_setup(void)
{
	/* Enable TIM2 clock. */
	rcc_periph_clock_enable(RCC_TIM2);
	rcc_periph_reset_pulse(RST_TIM2);
	/* Timer global mode: - No divider, Alignment edge, Direction up */
	timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT,
		       TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
	timer_continuous_mode(TIM2);
	timer_set_period(TIM2, PERIOD);
	timer_disable_oc_output(TIM2, TIM_OC2 | TIM_OC3 | TIM_OC4);
	timer_enable_oc_output(TIM2, TIM_OC1);
	timer_disable_oc_clear(TIM2, TIM_OC1);
	timer_disable_oc_preload(TIM2, TIM_OC1);
	timer_set_oc_slow_mode(TIM2, TIM_OC1);
	timer_set_oc_mode(TIM2, TIM_OC1, TIM_OCM_TOGGLE);
	timer_set_oc_value(TIM2, TIM_OC1, 50);
	timer_disable_preload(TIM2);
	/* Set the timer trigger output (for the DAC) to the channel 1 output
	   compare */
	timer_set_master_mode(TIM2, TIM_CR2_MMS_COMPARE_OC1REF);
	timer_enable_counter(TIM2);
}

/*--------------------------------------------------------------------*/
static void dma_setup(void)
{
	/* DAC channel 1 uses DMA controller 1 Stream 5 Channel 7. */
	/* Enable DMA1 clock and IRQ */
	rcc_periph_clock_enable(RCC_DMA1);

	/* Setup Stream5 Channel7 for DAC1 */
	nvic_enable_irq(NVIC_DMA1_STREAM5_IRQ);
	dma_stream_reset(DMA1, DMA_STREAM5);
	dma_set_priority(DMA1, DMA_STREAM5, DMA_SxCR_PL_LOW);
	dma_set_memory_size(DMA1, DMA_STREAM5, DMA_SxCR_MSIZE_8BIT);
	dma_set_peripheral_size(DMA1, DMA_STREAM5, DMA_SxCR_PSIZE_8BIT);
	dma_enable_memory_increment_mode(DMA1, DMA_STREAM5);
	dma_enable_circular_mode(DMA1, DMA_STREAM5);
	dma_set_transfer_mode(DMA1, DMA_STREAM5,
				DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
	/* The register to target is the DAC1 8-bit right justified data
	   register */
	dma_set_peripheral_address(DMA1, DMA_STREAM5, (uint32_t) &DAC_DHR8R1(DAC1));
	/* The array v[] is filled with the waveform data to be output */
	dma_set_memory_address(DMA1, DMA_STREAM5, (uint32_t) waveform);
	dma_set_number_of_data(DMA1, DMA_STREAM5, 1024);
	dma_enable_half_transfer_interrupt(DMA1, DMA_STREAM5);
	dma_enable_transfer_complete_interrupt(DMA1, DMA_STREAM5);
	dma_channel_select(DMA1, DMA_STREAM5, DMA_SxCR_CHSEL_7);
	dma_enable_stream(DMA1, DMA_STREAM5);


	/* Setup Stream6 Channel7 for DAC2 */
	//nvic_enable_irq(NVIC_DMA1_STREAM6_IRQ);
	dma_stream_reset(DMA1, DMA_STREAM6);
	dma_set_priority(DMA1, DMA_STREAM6, DMA_SxCR_PL_LOW);
	dma_set_memory_size(DMA1, DMA_STREAM6, DMA_SxCR_MSIZE_8BIT);
	dma_set_peripheral_size(DMA1, DMA_STREAM6, DMA_SxCR_PSIZE_8BIT);
	dma_enable_memory_increment_mode(DMA1, DMA_STREAM6);
	dma_enable_circular_mode(DMA1, DMA_STREAM6);
	dma_set_transfer_mode(DMA1, DMA_STREAM6,
				DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
	/* The register to target is the DAC2 8-bit right justified data
	   register */
	dma_set_peripheral_address(DMA1, DMA_STREAM6, (uint32_t) &DAC_DHR8R2(DAC1));
	/* The array v[] is filled with the waveform data to be output */
	dma_set_memory_address(DMA1, DMA_STREAM6, (uint32_t) waveform);
	dma_set_number_of_data(DMA1, DMA_STREAM6, 1024);
	//dma_enable_transfer_complete_interrupt(DMA1, DMA_STREAM6);
	dma_channel_select(DMA1, DMA_STREAM6, DMA_SxCR_CHSEL_7);
	dma_enable_stream(DMA1, DMA_STREAM6);

}

/*--------------------------------------------------------------------*/
static void dac_setup(void)
{
	/* Enable the DAC clock on APB1 */
	rcc_periph_clock_enable(RCC_DAC);

	/* Setup the DAC channel 1, with timer 2 as trigger source.
	 * Assume the DAC has woken up by the time the first transfer occurs */
	dac_trigger_enable(DAC1, DAC_CHANNEL1);
	dac_set_trigger_source(DAC1, DAC_CR_TSEL1_T2);
	dac_dma_enable(DAC1, DAC_CHANNEL1);
	dac_enable(DAC1, DAC_CHANNEL1);

	/* Setup the DAC channel 2, with timer 2 as trigger source.
	 * Assume the DAC has woken up by the time the first transfer occurs */
	dac_trigger_enable(DAC1, DAC_CHANNEL2);
	dac_set_trigger_source(DAC1, DAC_CR_TSEL2_T2);
	dac_dma_enable(DAC1, DAC_CHANNEL2);
	dac_enable(DAC1, DAC_CHANNEL2);

}

/*--------------------------------------------------------------------*/
/* The ISR simply provides a test output for a CRO trigger */

void dma1_stream5_isr(void)
{

	if (dma_get_interrupt_flag(DMA1, DMA_STREAM5, DMA_HTIF)) {
		dma_clear_interrupt_flags(DMA1, DMA_STREAM5, DMA_HTIF);
		/* Toggle LED to keep aware of activity and frequency. */
		gpio_set(GPIOA, GPIO8);
		wave_offset = gen_sin_wave(waveform,       512, wave_offset, 1024, wave_frequency, 0.5);
		//wave_offset = gen_square_wave(waveform,       512, wave_offset, 1024, wave_frequency, 0.1);
	}
	if (dma_get_interrupt_flag(DMA1, DMA_STREAM5, DMA_TCIF)) {
		dma_clear_interrupt_flags(DMA1, DMA_STREAM5, DMA_TCIF);
		/* Toggle LED to keep aware of activity and frequency. */
		gpio_clear(GPIOA, GPIO8);
		wave_offset = gen_sin_wave(waveform + 512, 512, wave_offset, 1024, wave_frequency, 0.5);
		//wave_offset = gen_square_wave(waveform + 512, 512, wave_offset, 1024, wave_frequency, 0.1);
		wave_delay++;
	}

	if (wave_delay == 100) {
		wave_delay = 0;
		wave_frequency += (0.1 * wave_direction);
		if (wave_frequency >= 2.0) {
			wave_direction = -1.0;
		}
		if (wave_frequency <= 1.0) {
			wave_direction = 1.0;
		}
	}
}

/*--------------------------------------------------------------------*/
int main(void)
{

	gen_sin_wave(waveform,       512,   0, 1024, 1.0, 0.5);
	gen_sin_wave(waveform + 512, 512, 512, 1024, 1.0, 0.5);

	clock_setup();
	gpio_setup();
	timer_setup();
	dma_setup();
	dac_setup();

	while(1) {
		asm("nop");
	}

	return 0;
}
