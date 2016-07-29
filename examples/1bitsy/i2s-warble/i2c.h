#ifndef I2C_included
#define I2C_included

#include <stdbool.h>
#include <stdint.h>

#include "gpio.h"

typedef struct i2c_config {
    uint32_t i_base_address;
    uint16_t i_own_address;
    gpio_pin i_pins[2];
} i2c_config;

typedef struct i2c_channel {
    uint32_t i_base_address;
    bool     i_is_master;
    bool     i_stop;
    uint16_t i_address;
} i2c_channel;

extern void init_i2c(const i2c_config *ip);

extern void i2c_transmit      (i2c_channel const *ip,
                               uint8_t const     *data,
                               size_t             count);
extern void i2c_receive       (const i2c_channel *ip,
                                   uint8_t       *data,
                                   size_t         count);

#endif /* !I2C_included */
