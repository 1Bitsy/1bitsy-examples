#ifndef SYSTICK_included
#define SYSTICK_included

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void systick_handler(uint32_t millis);

extern volatile uint32_t system_millis;


extern void setup_systick(uint32_t cpu_freq);

extern void register_systick_handler(systick_handler *);

extern void delay_msec(uint32_t msec);

#ifdef __cplusplus
}
#endif

#endif /* !SYSTICK_included */
