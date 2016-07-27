#ifndef I2C_included
#define I2C_included

#define AN2824_STYLE

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

#ifdef ARDUINO_STYLE

extern void init_i2c(const i2c_config *ip);

extern void i2c_begin_transmission(uint32_t interface, uint8_t address);
extern void i2c_end_transmission(uint32_t interface, bool stop);

extern void i2c_request_from(uint32_t interface, uint8_t address,
                             uint32_t count, bool stop);
extern void i2c_is_available(uint32_t interface);
extern void i2c_write(uint32_t interface, uint8_t value);
extern uint8_t i2c_read(uint32_t interface);

#endif

#ifdef AN2824_STYLE

extern void init_i2c(const i2c_config *ip);

extern void i2c_transmit      (i2c_channel const *ip,
                               uint8_t const     *data,
                               size_t             count);
extern void i2c_receive       (const i2c_channel *ip,
                                   uint8_t       *data,
                                   size_t         count);

#endif

#endif /* !I2C_included */
