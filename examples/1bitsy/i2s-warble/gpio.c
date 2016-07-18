#include "gpio.h"

#include <assert.h>
#include <stdbool.h>

#include <libopencm3/stm32/rcc.h>

#define GPIO_PORT_COUNT 11

static uint16_t gpio_pins_used[GPIO_PORT_COUNT];

void gpio_init_pin(const gpio_pin *pin)
{
    uint32_t port = pin->gp_port;
    uint16_t pinmask = pin->gp_pin;
    uint32_t index = ((uint32_t)port - (uint32_t)PERIPH_BASE_AHB1) >> 10;
    assert(index < GPIO_PORT_COUNT);

    if (!gpio_pins_used[index])
        rcc_periph_clock_enable((0x30 << 5) | index);

    assert(!(gpio_pins_used[index] & pinmask));
    gpio_pins_used[index] |= pinmask;

    gpio_mode_setup(port,
                    pin->gp_mode,
                    pin->gp_pupd,
                    pinmask);

    if (pin->gp_mode == GPIO_MODE_OUTPUT) {
        if (pin->gp_level)
            gpio_set(port, pinmask);
        else
            gpio_clear(port, pinmask);
    }

    if (pin->gp_mode == GPIO_MODE_OUTPUT || pin->gp_mode == GPIO_MODE_AF)
        gpio_set_output_options(port,
                                pin->gp_otype,
                                pin->gp_ospeed,
                                pinmask);

    if (pin->gp_mode == GPIO_MODE_AF)
        gpio_set_af(port,
                    pin->gp_af,
                    pinmask);
}

void gpio_init_pins(const gpio_pin *pins, size_t count)
{
    for (size_t i = 0; i < count; i++)
        gpio_init_pin(&pins[i]);
}
