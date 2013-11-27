#include <mchck.h>
#include "nrf.h"
// #include <usb/usb.h>
// #include <usb/cdc-acm.h>

static struct timeout_ctx t;
// static struct cdc_ctx cdc;

// static void
// new_data(uint8_t *data, size_t len)
// {
// 	// onboard_led(ONBOARD_LED_TOGGLE);
// 	uint8_t reg = data[0] - 97;
// 	uint8_t value = nrf_read_register_byte(reg);
// 	onboard_led(ONBOARD_LED_TOGGLE);
// 	printf("reg 0x%x => 0x%x\r\n", reg, value);
// 	cdc_read_more(&cdc);
// }

// static void
// init_vcdc(int config)
// {
// 	cdc_init(new_data, NULL, &cdc);
// 	cdc_set_stdout(&cdc);
// }

// static const struct usbd_device cdc_device =
// 	USB_INIT_DEVICE(0x2323,              /* vid */
// 					3,                   /* pid */
// 					u"mchck.org",        /* vendor */
// 					u"nrf test",         /* product" */
// 					(init_vcdc,          /* init */
// 					 CDC)                /* functions */
// 	);

static void
test(void *data)
{
	uint8_t value = nrf_read_register_byte(0x0);
	if (value == 0x8)
		onboard_led(ONBOARD_LED_TOGGLE);
}

void
main(void)
{
	nrf_init();
	timeout_add(&t, 200, test, NULL);
	// usb_init(&cdc_device);
	sys_yield_for_frogs();
}
