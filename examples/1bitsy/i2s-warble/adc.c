#include "adc.h"

#include <libopencm3/stm32/adc.h>

void init_adc_channel(const adc_channel *adc)
{
    gpio_init_pin(&adc->adc_gpio);

    adc_power_off(adc->adc_adc);
    adc_disable_scan_mode(adc->adc_adc);
    adc_set_sample_time_on_all_channels(adc->adc_adc, ADC_SMPR_SMP_3CYC);
    adc_power_on(adc->adc_adc);
}

uint16_t adc_read_single(const adc_channel *adc)
{
    uint8_t channel_array[16];
    channel_array[0] = adc->adc_channel;
    adc_set_regular_sequence(adc->adc_adc, 1, channel_array);
    adc_start_conversion_regular(adc->adc_adc);
    while (!adc_eoc(adc->adc_adc))
        continue;
    uint16_t reg16 = adc_read_regular(adc->adc_adc);
    return reg16;
}
