#include <mchck.h>
#include "nRF24L01plus.h"

static struct timeout_ctx t;
#define READ_TEMP_PERIOD 500

static void
temp_done(uint16_t data, int error, void *cbdata)
{
	static accum temp_deg;
	onboard_led(ONBOARD_LED_TOGGLE);
	unsigned accum volt = adc_as_voltage(data);
	accum volt_diff = volt - 0.719k;
	accum temp_diff = volt_diff * (1000K / 1.715K);
    temp_deg = 25k - temp_diff;
	nrf_btle_advertise(&temp_deg, sizeof(accum));
}

static void
read_temp(void *data)
{
	adc_sample_start(ADC_TEMP, temp_done, NULL);
	timeout_add(&t, READ_TEMP_PERIOD, read_temp, NULL);
}

void
main(void)
{
	timeout_init();
    adc_init();
    nrf_init();
    nrf_btle_init("Temp", 4);
    read_temp(NULL);
    sys_yield_for_frogs();
}
