#include <stdio.h>
#include <usb/usb.h>

static void init_my_hid(int config);

#include "hid_impl.h"
#include "desc.h"

static struct hid_ctx hid_ctx;

static int
report_in(uint8_t report_id, void *data_out, size_t data_size)
{
	printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ REPORT IN CALLED");
}

static int
report_out(uint8_t report_id, void *data_out, size_t data_size)
{
	printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ REPORT OUT CALLED");
}

static void
init_my_hid(int config)
{
	hid_descriptor.report_input = report_in;
	hid_descriptor.report_output = report_out;
	hid_init(&hid_descriptor, &hid_ctx);
}

int
main(void)
{
	usb_init(&hid_dev);
	vusb_main_loop();
}
