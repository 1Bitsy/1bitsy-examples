/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2016 Piotr Esden-Tempski <piotr@esden.net>
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
