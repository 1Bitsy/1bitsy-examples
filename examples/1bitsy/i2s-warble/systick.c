#include "systick.h"

#include <assert.h>

#include <libopencm3/cm3/systick.h>

volatile uint32_t system_millis;
static systick_handler *current_handler;

// This is the systick ISR.
void sys_tick_handler(void);
void sys_tick_handler(void)
{
    system_millis++;
    if (current_handler)
        (*current_handler)(system_millis);
}

void setup_systick(uint32_t cpu_freq)
{
    // set tick rate to 1 KHz.
    systick_set_reload(cpu_freq / 1000);
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
    systick_counter_enable();
    systick_interrupt_enable();
}

void register_systick_handler(systick_handler *handler)
{
    assert(!current_handler);
    current_handler = handler;
}

void delay_msec(uint32_t msec)
{
    uint32_t t0 = system_millis;
    while ((system_millis - t0) < msec)
        continue;
}
