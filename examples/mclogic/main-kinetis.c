#include <mchck.h>
#include "sump.h"
#include "mclogic.desc.h"

static struct cdc_ctx cdc;

static void
new_data(uint8_t *data, size_t len)
{
        onboard_led(-1);
        sump_process(data, len);
        cdc_read_more(&cdc);
}

static void
write_data(const uint8_t *buf, size_t len)
{
	cdc_write(buf, len, &cdc);
}

void
init_vcdc(int config)
{
	cdc_init(new_data, NULL, &cdc);
}

void
main(void)
{
	usb_init(&cdc_device);
	sump_init(&write_data);
        sys_yield_for_frogs();
}

/* vim: set ts=8 sw=8 noexpandtab: */
