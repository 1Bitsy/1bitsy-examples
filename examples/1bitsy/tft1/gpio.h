#ifndef GPIO_included
#define GPIO_included

#include <stddef.h>

#include <libopencm3/stm32/gpio.h>

typedef struct gpio_pin {
    uint32_t gp_port;           // GPIOA .. GPIOF
    uint16_t gp_pin;            // GPIO0 .. GPIO15
    uint8_t  gp_mode   : 2;     // GPIO_MODE_INPUT/OUTPUT/AF/ANALOG
    uint8_t  gp_pupd   : 2;     // GPIO_PUPD_NONE/PULLUP/PULLDOWN
    uint8_t  gp_af     : 4;     // GPIO_AF0 .. GPIO_AF15
    uint8_t  gp_ospeed : 2;     // GPIO_OSPEED_2/25/60/100MHZ
    uint8_t  gp_otype  : 1;     // GPIO_OTYPE_PP/OD (push-pull, open drain)
    uint8_t  gp_level  : 1;     // 0 or 1
} gpio_pin;

extern void gpio_init_pin(const gpio_pin *);

extern void gpio_init_pins(const gpio_pin *, size_t count);

#endif /* !GPIO_included */
