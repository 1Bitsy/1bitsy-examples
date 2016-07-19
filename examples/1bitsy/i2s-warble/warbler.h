#ifndef WARBLER_included
#define WARBLER_included

#include <stdbool.h>
#include <stdint.h>

typedef enum warbler_state {
    WS_IDLE,
    WS_RISING,
    WS_FALLING,
} warbler_state;

typedef struct warbler {
    warbler_state w_state;
    float         w_f0;
    float         w_f1;
    float         w_phase;
    float         w_dphase;
    float         w_ddphase;
    uint32_t      w_samp_no;
    uint32_t      w_cyc_no;
} warbler;

extern void    init_warbler(warbler *wp, float f0, float f1);

extern bool    warbler_is_active(const warbler *wp);

extern void    warbler_trigger(warbler *wp, warbler_state s, float slew);
extern int16_t warbler_next_sample(warbler *wp, float slew);

#endif /* !WARBLER_included */
