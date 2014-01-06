#include <mchck.h>
#include "blink.h"
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

void
main(void)
{
	blink_init(100);
	blink(2);
	sump_init(write_data);
	usb_init(&cdc_device);
	sys_yield_for_frogs();
}
