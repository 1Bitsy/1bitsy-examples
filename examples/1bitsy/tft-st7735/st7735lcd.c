/*
 * Copyright (C) 2017 Mark Osborne @BecomingMaker
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
#include "systick.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>

// 1bitsy pins connected to TFT
#define TFT_SCK GPIO10
#define TFT_MOSI GPIO15
#define TFT_NSS GPIO12
#define TFT_RESET GPIO11
#define TFT_DC GPIO1

// TFT Commands
#define SWRESET 0x01    // software reset
#define SLPOUT  0x11    // sleep out
#define DISPOFF 0x28    // display off
#define DISPON  0x29    // display on
#define CASET   0x2A    // column address set
#define RASET   0x2B    // row address set
#define RAMWR   0x2C    // RAM write
#define MADCTL  0x36    // axis control
#define COLMOD  0x3A    // color mode

// 1.8" TFT display constants
#define XSIZE   128
#define YSIZE   160
#define XMAX    XSIZE-1
#define YMAX    YSIZE-1

// Color constants
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x0400
#define LIME    0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

static void systick_setup(void)
{
    rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_168MHZ]);
    setup_systick(rcc_ahb_frequency);
}

static void gpio_setup(void)
{
    // Enable GPIOB clock
    rcc_periph_clock_enable(RCC_GPIOB);

    // TFT Reset pin, pull low for 10uS or more to initiate reset
    // Reset takes up to 120ms
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, TFT_RESET);
    gpio_set(GPIOB, TFT_RESET);

    // TFT A0 - D/C pin, 0:Command 1:Data
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, TFT_DC);
    gpio_clear(GPIOB, TFT_DC);
}

static void spi_init(void)
{
    // Set pin mode for SPI managed pins to alternate function
    gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE,
        TFT_SCK		  // SCK - serial clock
        | TFT_MOSI	  // MOSI - master out slave in
        | TFT_NSS	  // NSS - slave select
    );

    // Set alternate function for SPI managed pins to AF5 for SPI2
    gpio_set_af(GPIOB, GPIO_AF5,
        TFT_SCK       // SPI2_SCK
        | TFT_MOSI    // SPI2_MOSI
        | TFT_NSS     // SPI2_NSS
    );

    // Enable SPI periperal clock
    rcc_periph_clock_enable(RCC_SPI2);

    // Initialize SPI2 as master
    spi_init_master(
        SPI2,
        SPI_CR1_BAUDRATE_FPCLK_DIV_2,
        SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,   // CPOL: Clock low when idle
        SPI_CR1_CPHA_CLK_TRANSITION_1,     // CPHA: Clock phase: read on rising edge of clock
        SPI_CR1_DFF_8BIT,
        SPI_CR1_MSBFIRST);

    spi_disable_crc(SPI2);

    // Have SPI peripheral manage NSS pin (pulled low when SPI enabled)
    spi_enable_ss_output(SPI2);

    spi_enable(SPI2);
}

static void tft_hardware_reset(void)
{
    gpio_clear(GPIOB,TFT_RESET); // reset line must be pulled low for a minumum of 10uS
    delay_msec(1);
    gpio_set(GPIOB,TFT_RESET);	 // hardware reset can take up to 120ms
    delay_msec(120);
}


static void tft_send(uint8_t cmd)
{
    // We use spi_xfer here and below because it blocks waiting for transmission to complete
    // We ignore the unrequired read result
    spi_xfer(SPI2, cmd);

    // An alternative approach is to use spi_send and wait for transfer to finish
    // spi_send(SPI2, cmd);
    // while (!(SPI_SR(SPI2) & SPI_SR_TXE));
    // while ((SPI_SR(SPI2) & SPI_SR_BSY));
}

static void tft_write_cmd (uint8_t cmd)
{
    gpio_clear(GPIOB, TFT_DC);      // Set DC low to indicate command
    tft_send(cmd);
}

static void tft_write_byte (uint8_t data)
{
    gpio_set(GPIOB,TFT_DC);         // set DC high to indicate data
    tft_send(data);
}

static void tft_write_word (uint16_t data)
{
    // We send a word as two bytes as we initialized SPI in 8 bit mode
    gpio_set(GPIOB, TFT_DC);    // set DC high to indicate data
    tft_send(data >> 8);	    // send MSB byte
    tft_send(data & 0x00FF);	// send LSB
}

static void tft_init(void)
{
    // Hardware reset TFT first to ensure it is in a good state after power on
    tft_hardware_reset();

    // Take TFT out of sleep mode
    tft_write_cmd(SLPOUT);      // Must wait 120 ms after Sleep out command
    delay_msec(120);

    // Select 16 bit 565 color (default on reset is 18 bit)
    tft_write_cmd(COLMOD);
    tft_write_byte(0x05);

    // Turn the display on
    tft_write_cmd(DISPON);
}


/*
 * Defines a block of pixels that the following color data will be written to
 * A block could be as small as a single pixel or as large as the entire screen
 */
static void tft_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    tft_write_cmd(CASET);   // set column range (x0,x1)
    tft_write_word(x0);
    tft_write_word(x1);
    tft_write_cmd(RASET);   // set row range (y0,y1)
    tft_write_word(y0);
    tft_write_word(y1);
}

/*
 * Example graphics function to fill a rectangle with a single color
 */
static void fill_rect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    // Compute number of pixels in rectange
    uint16_t width = x1-x0+1;
    uint16_t height = y1-y0+1;
    uint16_t size = width * height;

    // Define address window and write pixel color every pixel in the window
    tft_set_window(x0,y0,x1,y1);
    tft_write_cmd(RAMWR);
    for (uint16_t i=size; i>0; --i)
    {
        tft_write_word(color);
    }
}

int main(void)
{
    // Setup
    button_boot();
    systick_setup();
    gpio_setup();
    spi_init();
    tft_init();

    // Set entire screen to black
    fill_rect(0,0,XMAX,YMAX,BLACK);

    // Loop
    while (1) {
        fill_rect(20,20,XMAX-20,YMAX-20,BLUE);
        delay_msec(1000);

        fill_rect(20,20,XMAX-20,YMAX-20,GREEN);
        delay_msec(1000);
    }

    return 0;
}
