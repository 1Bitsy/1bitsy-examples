#include "sgtl5000.h"

#include <assert.h>
#include <stdio.h>

#include "systick.h"

#define CHIP_ID            0x0000
#define CHIP_DIG_POWER     0x0002
#define CHIP_CLK_CTRL      0x0004
#define CHIP_I2S_CTRL      0x0006
#define CHIP_SSS_CTRL      0x000A
#define CHIP_ADCDAC_CTRL   0x000E
#define CHIP_DAC_VOL       0x0010
#define CHIP_PAD_STRENGTH  0x0014
#define CHIP_ANA_ADC_CTRL  0x0020
#define CHIP_ANA_HP_CTRL   0x0022
#define CHIP_ANA_CTRL      0x0024
#define CHIP_LINREG_CTRL   0x0026
#define CHIP_REF_CTRL      0x0028
#define CHIP_MIC_CTRL      0x002A
#define CHIP_LINE_OUT_CTRL 0x002C
#define CHIP_LINE_OUT_VOL  0x002E
#define CHIP_ANA_POWER     0x0030
#define CHIP_PLL_CTRL      0x0032
#define CHIP_CLK_TOP_CTRL  0x0034
#define CHIP_ANA_STATUS    0x0036
#define CHIP_ANA_TEST1     0x0038
#define CHIP_ANA_TEST2     0x003A
#define CHIP_SHORT_CTRL    0x003C

static void sgtl_write_register(i2c_channel const *i2c,
                                uint16_t           reg,
                                uint16_t           value)
{
    uint8_t buf[4] = { reg >> 8, reg & 0xFF, value >> 8, value & 0xFF };
    i2c_transmit(i2c, buf, 4);
}

static uint16_t sgtl_read_register(i2c_channel const *i2c, uint16_t reg)
{
    uint8_t out[2] = { reg >> 8, reg & 0xFF };
    uint8_t in[2] = { 0xFF, 0xFF };
    i2c_transmit(i2c, out, 2);
    i2c_receive(i2c, in, 2);
    return in[0] << 8 | in[1];
}

// I2C command sequence cribbed from Teensy Audio Library

void init_sgtl5000(const i2c_channel *i2c)
{
    delay_msec(5);

    for (uint8_t i = 0; i < 10; i++) {
        uint16_t id = sgtl_read_register(i2c, CHIP_ID);
        fprintf(stderr, "sgtl: chip ID = %#x\n", id);
        if (id == 0xA011)
            break;
    }

    enum { DELAY = 0xFFFF };

    static const uint16_t reg_loads[][2] = {
        { CHIP_ANA_POWER,     0x4060 }, // VDDD is externally driven with 1.8V
        { CHIP_LINREG_CTRL,   0x006C }, // VDDA & VDDIO both over 3.1V
        { CHIP_REF_CTRL,      0x01F2 }, // VAG=1.575, normal ramp,
                                        // +12.5% bias current
        { CHIP_LINE_OUT_CTRL, 0x0F22 }, // LO_VAGCNTRL=1.65V, OUT_CURRENT=0.54mA
        { CHIP_SHORT_CTRL,    0x4446 }, // allow up to 125mA
        { CHIP_ANA_CTRL,      0x0137 }, // enable zero cross detectors
        { CHIP_ANA_POWER,     0x40FF }, // power up: lineout, hp, adc, dac
        { CHIP_DIG_POWER,     0x0073 }, // power up all digital stuff
        { DELAY,                 400 }, // Delay 400 msec
        { CHIP_LINE_OUT_VOL,  0x1D1D }, // default approx 1.3V P-P
        { CHIP_CLK_CTRL,      0x0004 }, // 44.1 kHz, 256*Fs
        { CHIP_I2S_CTRL,      0x0130 }, // SCLK=32*Fs, 16bit, I2S format
                                        // default signal routing is ok?
        { CHIP_SSS_CTRL,      0x0010 }, // ADC->I2S, I2S->DAC
        { CHIP_ADCDAC_CTRL,   0x0000 }, // disable dac mute
        { CHIP_DAC_VOL,       0x3C3C }, // digital gain, 0dB
        { CHIP_ANA_HP_CTRL,   0x7F7F }, // set volume (lowest level)
        { CHIP_ANA_CTRL,      0x0026 }, // enable zero cross detectors
    };
    static const size_t reg_load_count = (&reg_loads)[1] - reg_loads;

    for (size_t i = 0; i < reg_load_count; i++) {
        uint16_t reg = reg_loads[i][0];
        uint16_t val = reg_loads[i][1];

        if (reg == DELAY)
            delay_msec(val);
        else
            sgtl_write_register(i2c, reg, val);
    }
}

void sgtl_set_volume(const i2c_channel *i2c, float left, float right)
{
    uint8_t l = 0x18 - 2 * left;
    uint8_t r = 0x18 - 2 * right;
    printf("l = %u; r = %u\n", l, r);
    if (l > 0x3F)
        l = 0x3F;               // clamp out of range to min.
    if (r > 0x3F)
        r = 0x3F;

    sgtl_write_register(i2c, CHIP_ANA_HP_CTRL, r << 8 | l);
}

