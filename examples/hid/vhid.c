#include <stdio.h>
#include <usb/usb.h>

static void init_my_hid(int config);

#include "hid_impl.h"
#include "desc.h"

static struct hid_ctx hid_ctx;

static void
init_my_hid(int config)
{
	hid_init(&hid_descriptor, &hid_ctx);
}

int
main(void)
{
	usb_init(&hid_dev);
	vusb_main_loop();
}
