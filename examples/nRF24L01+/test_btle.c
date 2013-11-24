#include <mchck.h>
#include "nRF24L01plus.h"

static struct timeout_ctx t;
static volatile int temp_cnt = -1; // give us some time
#define READ_TEMP_PERIOD 500

static void
temp_done(uint16_t data, int error, void *cbdata)
{
	onboard_led(ONBOARD_LED_TOGGLE);
	static unsigned accum volt;
	volt = adc_as_voltage(data);
	nrf_btle_advertise(&volt, sizeof(unsigned accum));
	if (--temp_cnt > 0) // when timeout too short
		adc_sample_start(ADC_TEMP, temp_done, NULL);
}

static void
read_temp(void *data)
{
	if (temp_cnt++ == 0)
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
