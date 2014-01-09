#include <mchck.h>
#include "sump.h"
#include "mclogic.desc.h"

static struct cdc_ctx cdc;

static void
new_data(uint8_t *data, size_t len)
{
	sump_process(data, len);
	cdc_read_more(&cdc);
}

static ssize_t
write_data(const uint8_t *buf, size_t len)
{
	return cdc_write(buf, len, &cdc);
}

void
init_vcdc(int config)
{
	cdc_init(new_data, sump_data_sent, &cdc);
}

#ifdef MCLOGIC_TEST
static void
pit_handler_fakeclk(enum pit_id id)
{
	gpio_toggle(PIN_PTC4);
}
#endif

void
main(void)
{
	sump_init(write_data);
	usb_init(&cdc_device);
#ifdef MCLOGIC_TEST
	gpio_dir(PIN_PTC4, GPIO_OUTPUT);
	pit_start(PIT_3, (48 * 10) - 1, pit_handler_fakeclk);
#endif
	sys_yield_for_frogs();
}
