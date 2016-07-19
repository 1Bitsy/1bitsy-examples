#include "warbler.h"

#include <math.h>

#define Fs     44100.0          // sample rate: Hz
#define IVL        0.5          // sec
#define NCYCLE    20            // cycles

const uint32_t UP_SAMPLES = (uint32_t)(Fs * IVL + 0.5);
const uint32_t DN_SAMPLES = (uint32_t)(Fs * IVL + 0.5);

void init_warbler(warbler *wp, float f0, float f1)
{
    wp->w_state   = WS_IDLE;
    wp->w_f0      = f0;
    wp->w_f1      = f1;
    wp->w_phase   = 0.0;
    wp->w_dphase  = 0.0;
    wp->w_ddphase = 1.0;
    wp->w_samp_no = 0;
    wp->w_cyc_no  = 0;
}

bool warbler_is_active(const warbler *wp)
{
    return wp->w_state != WS_IDLE;
}

void warbler_trigger(warbler *wp, warbler_state s, float slew)
{
    wp->w_state = s;
    switch (s) {

    case WS_IDLE:
        wp->w_phase   = 0;
        wp->w_dphase  = 0.0;
        wp->w_ddphase = 1.0;
        wp->w_samp_no = 0;
        wp->w_cyc_no  = 0;
        break;

    case WS_RISING:
        wp->w_dphase  = wp->w_f0 / Fs;
        wp->w_ddphase = powf(wp->w_f1 / wp->w_f0, 1 / (slew * Fs));
        wp->w_samp_no = 0;
        wp->w_cyc_no  = 0;
        break;

    case WS_FALLING:
        wp->w_dphase  = wp->w_f1 / Fs;
        wp->w_ddphase = powf(wp->w_f0 / wp->w_f1, 1 / (slew * Fs));
        wp->w_samp_no = 0;
        wp->w_cyc_no  = 0;
        break;
    }
}

int16_t warbler_next_sample(warbler *wp, float slew)
{
    int16_t samp = wp->w_phase * 32768 - 16384;
    wp->w_phase += wp->w_dphase;
    wp->w_dphase *= wp->w_ddphase;

    switch (wp->w_state) {

    case WS_IDLE:
        samp = 0;
        break;

    case WS_RISING:
        if (wp->w_dphase > wp->w_f1 / Fs) {
            wp->w_dphase = wp->w_f1 / Fs;
            wp->w_ddphase = 1.0;
        }
        if (++wp->w_samp_no >= UP_SAMPLES) {
            wp->w_samp_no = 0;
            if (++wp->w_cyc_no == NCYCLE) {
                wp->w_state = WS_IDLE;
                break;
            }
            wp->w_state = WS_FALLING;
            if (slew) {
                wp->w_ddphase = powf(wp->w_f0 / wp->w_f1, 1 / (slew * Fs));
            } else {
                wp->w_dphase  = wp->w_f0 / Fs;
                wp->w_ddphase = 1.0;
            }
        }
        break;

    case WS_FALLING:
        if (wp->w_dphase < wp->w_f0 / Fs) {
            wp->w_dphase = wp->w_f0 / Fs;
            wp->w_ddphase = 1.0;
        }
        if (++wp->w_samp_no >= DN_SAMPLES) {
            wp->w_samp_no = 0;
            if (++wp->w_cyc_no == NCYCLE) {
                wp->w_state = WS_IDLE;
                break;
            }
            wp->w_state = WS_RISING;
            if (slew) {
                wp->w_ddphase = powf(wp->w_f1 / wp->w_f0, 1 / (slew * Fs));
            } else {
                wp->w_dphase  = wp->w_f1 / Fs;
                wp->w_ddphase = 1.0;
            }
        }
        break;
    }

    return samp;
}
