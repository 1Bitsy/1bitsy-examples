/*
 * This file is part of the 1bitsy project.
 *
 * Copyright (C) 2017 Piotr Esden-Tempski <piotr@esden.net>
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

#include <stdint.h>
#include <stdio.h>

#include "uart.h"

enum {
    RDI_SYS_OPEN = 0x01,
    RDI_SYS_WRITE = 0x05,
    RDI_SYS_ISTTY = 0x09,
};

static int rdi_write(int fn, const char *buf, size_t len)
{

    (void)fn;

    /* in case we implement other means of output here they go: */
    return len - uart_write_blocking(buf, len);
}

struct ex_frame {
    union {
        int syscall;
        int retval;
    };
    const int *params;
    uint32_t r2, r3, r12, lr, pc;
};

void debug_monitor_handler_c(struct ex_frame *sp);

void debug_monitor_handler_c(struct ex_frame *sp)
{
    /* Return to after breakpoint instruction */
    sp->pc += 2;

    switch (sp->syscall) {
    case RDI_SYS_OPEN:
        sp->retval = 1;
            break;
    case RDI_SYS_WRITE:
        sp->retval = rdi_write(sp->params[0], (void*)sp->params[1], sp->params[2]);
        break;
    case RDI_SYS_ISTTY:
        sp->retval = 1;
        break;
    default:
        sp->retval = -1;
    }

}

asm(".globl debug_monitor_handler\n"
    ".thumb_func\n"
    "debug_monitor_handler: \n"
    "    mov r0, sp\n"
    "    b debug_monitor_handler_c\n");