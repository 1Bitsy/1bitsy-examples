#ifndef ADC_included
#define ADC_included

#include <stdint.h>

#include "gpio.h"

typedef struct adc_channel {
    uint32_t adc_adc;
    uint8_t adc_channel;
    gpio_pin adc_gpio;
} adc_channel;

extern void init_adc_channel(const adc_channel *);

extern uint16_t adc_read_single(const adc_channel *);

#endif /* !ADC_included */
