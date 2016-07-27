##
## This file is part of the libopencm3 project.
##
## Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
##
## This library is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License
## along with this library.  If not, see <http://www.gnu.org/licenses/>.
##

BINARY := i2s-warble

SOURCES := adc.c console.c debounce.c gpio.c i2c.c i2s.c systick.c warbler.c
OBJS := $(patsubst %.cpp, %.o, $(SOURCES:.c=.o))

LDSCRIPT := ../stm32f4-1bitsy.ld

include ../Makefile.include

.PHONY: tags
tags:
	make-etags-SimplicityStudio `strings -a "$(LIB_DIR)/libopencm3_stm32f4.a" | grep '\.\./.*\.c' | sed 's,^\([^/]\),../../../../../libopencm3/lib/stm32/f4/\1,'`
