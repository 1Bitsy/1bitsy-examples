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
