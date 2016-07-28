#ifndef I2S_included
#define I2S_included

#include <stdbool.h>
#include <stdint.h>

#include <libopencm3/stm32/spi.h>

#include "gpio.h"

typedef enum i2s_mode {
    I2SM_SLAVE_TX    = SPI_I2SCFGR_I2SCFG_SLAVE_TRANSMIT,
    I2SM_SLAVE_RX    = SPI_I2SCFGR_I2SCFG_SLAVE_RECEIVE,
    I2SM_MASTER_TX   = SPI_I2SCFGR_I2SCFG_MASTER_TRANSMIT,
    I2SM_MASTER_RX   = SPI_I2SCFGR_I2SCFG_MASTER_RECEIVE,
} __attribute__((__packed__)) i2s_mode;

typedef enum i2s_standard {
    I2SS_PHILIPS     = SPI_I2SCFGR_I2SSTD_I2S_PHILIPS,
    I2SS_MSB         = SPI_I2SCFGR_I2SSTD_MSB_JUSTIFIED,
    I2SS_LSB         = SPI_I2SCFGR_I2SSTD_LSB_JUSTIFIED,
    I2SS_PCM_SHORT   = SPI_I2SCFGR_I2SSTD_PCM,
    I2SS_PCM_LONG    = SPI_I2SCFGR_I2SSTD_PCM | 0x8,
} __attribute__((__packed__)) i2s_standard;

typedef enum i2s_data_format {
    I2SF_16          = SPI_I2SCFGR_DATLEN_16BIT << SPI_I2SCFGR_DATLEN_LSB,
    I2SF_16_EXTENDED = SPI_I2SCFGR_DATLEN_16BIT << SPI_I2SCFGR_DATLEN_LSB
                     | SPI_I2SCFGR_CHLEN,
    I2SF_24          = SPI_I2SCFGR_DATLEN_24BIT << SPI_I2SCFGR_DATLEN_LSB
                     | SPI_I2SCFGR_CHLEN,
    I2SF_32          = SPI_I2SCFGR_DATLEN_32BIT << SPI_I2SCFGR_DATLEN_LSB
                     | SPI_I2SCFGR_CHLEN,
} __attribute__((__packed__)) i2s_data_format;

typedef enum i2s_mclk_mode {
    I2SM_DISABLED    = 0,
    I2SM_ENABLED     = SPI_I2SPR_MCKOE,
} i2s_mclk_mode;

typedef enum i2s_clock_polarity {
    I2SC_CPOL_LOW    = 0,
    I2SC_CPOL_HIGH   = SPI_I2SCFGR_CKPOL,
} __attribute__((__packed__)) i2s_clock_polarity;

typedef enum i2s_clock_source {
    I2SC_PLL,
    I2SC_EXTERNAL,
} __attribute__((__packed__)) i2s_clock_source;

typedef struct i2s_config {
    uint32_t           i2sc_sample_frequency;
    i2s_mode           i2sc_mode;
    i2s_standard       i2sc_standard;
    i2s_data_format    i2sc_data_format;
    i2s_mclk_mode      i2sc_mclk_output;
    i2s_clock_polarity i2sc_cpol;
    i2s_clock_source   i2sc_clock_source;
    bool               i2sc_full_duplex;
    gpio_pin           i2sc_gpio_pins[5];
} __attribute__((__packed__)) i2s_config;

typedef struct i2s_instance {
    uint32_t           i2si_base_address;
} __attribute__((__packed__)) i2s_instance;

extern void init_i2s(const i2s_config *, const i2s_instance *);

#endif /* !I2S_included */
