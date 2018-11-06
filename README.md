# README

[![Gitter channel](https://badges.gitter.im/libopencm3/discuss.svg)](https://gitter.im/1bitsy/Lobby)

This repository contains assorted example projects for the 1Bitsy development platform.

## Usage

To be able to compile the examples included in this repository you will require
an arm-none-eabi gcc toolchain. We recommend you install the gcc-arm-embedded
toolchain that is being maintained by ARM. The downloads and installation
instructions for Linux (Ubuntu), Mac OS X and Windows can be found on the
[gcc-arm-embedded Launchpad page](https://launchpad.net/gcc-arm-embedded/).

You _must_ run `make` in the top level directory first.  This builds the
library and all examples.  If you're simply hacking on a single example after
that, you can type `make clean; make` in any of the individual project
directories later.

For more verbose output, to see compiler command lines, use `make V=1`
For insanity levels of verboseness, use `make V=99`

The makefiles are generally usable for your own projects with
only minimal changes for the libopencm3 install path (See Reuse)

## Notes

If you are using an older GCC compiler (pre V5) you may need to specify the C
standard, otherwise the compiler might complain. To solve this
issue you can invoke `make` in the top level directory while setting
the `CFLAGS` variable:
```
CFLAGS=-std=c99 make
```

## Make Flash Target

To flash the 'miniblink' example to the 1Bitsy (after running `make` in the top-level directory), you can
execute:
```
cd examples/1bitsy/miniblink
make flash
```

The Makefiles for the examples are configured to use a Black Magic Probe by
default, and it will list the serial ports that it found for you to choose
from.

If you don't want to choose from a list, you can provide the serial port directly to the target by invoking:
```
cd examples/1bitsy/miniblink
make flash BMP_PORT=/dev/ttyACM0
```

You can also use `dfu-util` to upload the selected firmware by running:
```
cd examples/1bitsy/miniblink
make dfu-flash
```

## Flashing Manually

You can also flash manually using a myriad of different tools depending on
your setup. Here are a few examples.

### Black Magic Probe

```
cd examples/1bitsy/fancyblink
arm-none-eabi-gdb miniblink.elf
target extended-remote /dev/ttyACM0
monitor swdp_scan
attach 1
load
run
```

To exit the gdb session type `<Ctrl>-C` and `<Ctrl>-D`. It is useful to add the
following to the .gdbinit to make the flashing and debugging easier:
```
set target-async on
set confirm off
set mem inaccessible-by-default off
#set debug remote 1
tar ext /dev/ttyACM0
mon version
mon swdp_scan
att 1
```

Having this in your .gdbinit boils the flashing/debugging process down to:
```
cd examples/1bitsy/fancyblink
arm-none-eabi-gdb miniblink.elf
load
run
```

### DFU-Util

If you put your 1Bitsy in DFU Bootloader mode (shorting the DFU Bootloader
jumper while you plug in the USB cable, for example) you will be able to
use the following command to upload your firmware:
```
cd examples/1bitsy/fancyblink
make fancyblink.bin
sudo dfu-util -d 0483:df11 -c 1 -a 0 -s 0x08000000:leave -D fancyblink.bin
```

For this to work you will need `dfu-util` V0.8 or newer!

## Reuse

If you want to use any of the code found here in your own project, this
examples repository shows the general way. There is also a
1bitsy-locm3-template repository that can serve as a basis for your project.

1. Create an empty repository
```
mkdir mycoolrobot && cd mycoolrobot && git init .
```

2. Add libopencm3 as a submodule
```
git submodule add https://github.com/libopencm3/libopencm3
```

3. Grab a copy of the basic rules

These urls grab the latest from the libopencm3-examples repository.
```
wget \
  https://raw.githubusercontent.com/libopencm3/libopencm3-examples/master/examples/rules.mk \
  -O libopencm3.rules.mk
```

4. Grab a copy of the target Makefile

In this case, we want STM32F4.
```
wget \
  https://raw.githubusercontent.com/libopencm3/libopencm3-examples/master/examples/stm32/f4/Makefile.include \
  -O libopencm3.target.mk
```

5. Edit paths in `libopencm3.target.mk`

Edit the _last_ line of `libopencm3.target.mk` and change the include to read
`include ../libopencm3.rules.mk` (the amount of .. depends on where you put your
project in the next step).

6. Beg/borrow/steal an example project

For sanity's sake, use the same target as the makefile you grabbed above.
```
cp -a \
  somewhere/libopencm3-examples/examples/stm32/f4/stm32f4-discovery/miniblink \
  myproject
```

Add the path to `OPENCM3_DIR`, set the right `DEVICE` and modify the path to
the makefile include:
```
diff -u
--- Makefile.orig       2017-06-11 12:42:25.378248089 +0200
+++ Makefile    2017-06-11 12:46:42.566244271 +0200
@@ -19,7 +19,8 @@

BINARY = miniblink
 
-DEVICE=STM32F407VG
+OPENCM3_DIR = ../libopencm3
+DEVICE = STM32F415RGT6

-include ../../Makefile.include
+include ../libopencm3.target.mk
```

You're done :)

You need to run `make` inside the libopencm3 directory once to build the
library, then you can just run `make`/`make clean` in your project directory as
often as you like.
