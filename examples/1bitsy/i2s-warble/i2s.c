#include "i2s.h"

#include <assert.h>

#include <libopencm3/stm32/rcc.h>

extern void init_i2s(const i2s_config *cfg, const i2s_instance *inst)
{
    uint32_t base = inst->i2si_base_address;

    // Just handle one case for now.
    assert(cfg->i2sc_sample_frequency == 44100);
    assert(cfg->i2sc_mode             == I2SM_MASTER_TX);
    assert(cfg->i2sc_standard         == I2SS_LSB);
    assert(cfg->i2sc_data_format      == I2SF_16);
    assert(cfg->i2sc_mclk_output      == false);
    assert(cfg->i2sc_cpol             == I2SC_CPOL_HIGH);
    assert(cfg->i2sc_clock_source     == I2SC_PLL);
    assert(cfg->i2sc_full_duplex      == false);
    
    // Enable I2S clock.
    RCC_CR |= RCC_CR_PLLI2SON;

    SPI_I2SCFGR(base) = 0;
    
    uint32_t i2sdiv = 2;
    uint32_t packetlength = 1;
    uint32_t tmp = 0;
    uint32_t i2sclk = 0;

    packetlength = cfg->i2sc_data_format == I2SF_16 ? 1 : 2;
    if (cfg->i2sc_clock_source == I2SC_EXTERNAL) {
        if (!(RCC_CFGR & RCC_CFGR_I2SSRC))
            RCC_CFGR |= RCC_CFGR_I2SSRC;
        i2sclk = 12288000;      // internal clock frequency
    } else {
        assert(RCC_CR & RCC_CR_PLLI2SON);
        if (RCC_CFGR & RCC_CFGR_I2SSRC)
            RCC_CFGR &= ~RCC_CFGR_I2SSRC;
        if (RCC_PLLCFGR & RCC_PLLCFGR_PLLSRC) {
            // HSE selected
            // ??? Maybe 25MHz instead?
            i2sclk = 8000000 / (RCC_PLLCFGR & RCC_PLLCFGR_PLLM_MASK);
        } else {
            // HSI selected
            i2sclk = 16000000 / (RCC_PLLCFGR & RCC_PLLCFGR_PLLM_MASK);
        }
        i2sclk *= (RCC_PLLI2SCFGR & RCC_PLLI2SCFGR_PLLI2SN_MASK) >> 6;
        i2sclk /= (RCC_PLLI2SCFGR & RCC_PLLI2SCFGR_PLLI2SN_MASK) >> 28;
    }
    if (cfg->i2sc_mclk_output) {
        tmp = i2sclk / 256 * 10 / cfg->i2sc_sample_frequency + 5;
    } else {
        tmp = i2sclk /
            (32 * packetlength * 10) /
            (cfg->i2sc_sample_frequency + 5);
    }
    tmp /= 10;
    uint32_t i2sodd = (tmp & 1) << 8;
    i2sdiv = tmp / 2;
    if (i2sdiv < 2 || i2sdiv > 0xFF) {
        i2sdiv = 2;
        i2sodd = 0;
    }

    SPI_I2SPR(base) = i2sdiv | i2sodd | cfg->i2sc_mclk_output;
    SPI_I2SCFGR(base) = SPI_I2SCFGR_I2SMOD
                      | cfg->i2sc_mode << SPI_I2SCFGR_I2SCFG_LSB
                      | cfg->i2sc_standard << SPI_I2SCFGR_I2SSTD_LSB
                      | cfg->i2sc_data_format
                      | cfg->i2sc_cpol;

    if (cfg->i2sc_full_duplex) {
        assert(false && "Full duplex not implemented");
    }
}

// void reset_i2s(const i2s_instance *inst)
// {}
