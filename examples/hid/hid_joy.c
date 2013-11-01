#include <mchck.h>
#include <usb/usb.h>

static void init_my_hid_joy(int config);

#include "hid_impl.h"
#include "desc_joy.h"

static struct hid_ctx hid_ctx;

/* (taken from unojoy)
 * Series of bytes that appear in control packets right after the HID
 * descriptor is sent to the host. They where discovered by tracing output
 * from a Madcatz SF4 Joystick. Sending these bytes makes the PS button work.
 */
static const uint8_t magic_init_bytes[8] = {
	0x21, 0x26, 0x01, 0x07, 0x00, 0x00, 0x00, 0x00
};

static size_t
hid_get_descriptor(enum hid_report_descriptor_type type, uint8_t index, void **data_out)
{
	if (type != USB_HID_REPORT_DESC_TYPE_REPORT)
		return (0);
	*data_out = report_desc;
	return (REPORT_DESC_SIZE);
}

static size_t
hid_get_report(enum hid_report_type type, uint8_t report_id, void **data_out)
{
	if (type != USB_HID_REPORT_TYPE_INPUT)
		return (0);
	*data_out = &magic_init_bytes;
	return (8);
}

static struct hid_user_functions_t hid_funcs = {
	.get_descriptor = hid_get_descriptor,
	.get_report = hid_get_report,
};

static void
buttons_poll()
{
	joystick_data.square_btn = !gpio_read(GPIO_PTD0);
	joystick_data.cross_btn = !gpio_read(GPIO_PTD1);
	joystick_data.circle_btn = !gpio_read(GPIO_PTD2);
	joystick_data.triangle_btn = !gpio_read(GPIO_PTD3);
	joystick_data.l1_btn = !gpio_read(GPIO_PTD4);
	joystick_data.r1_btn = !gpio_read(GPIO_PTD5);
	joystick_data.l2_btn = !gpio_read(GPIO_PTD6);
	joystick_data.r2_btn = !gpio_read(GPIO_PTD7);
	joystick_data.select_btn = !gpio_read(GPIO_PTA1);
	joystick_data.start_btn = !gpio_read(GPIO_PTA2);
	joystick_data.l3_btn = !gpio_read(GPIO_PTA4);
	joystick_data.r3_btn = !gpio_read(GPIO_PTA18);
	joystick_data.ps_btn = !gpio_read(GPIO_PTA19);
	// axis
	joystick_data.left_y = !gpio_read(GPIO_PTC4) ? 127 : (!gpio_read(GPIO_PTC6) ? -127 : 0);
	joystick_data.left_x = !gpio_read(GPIO_PTC5) ? 127 : (!gpio_read(GPIO_PTC7) ? -127 : 0);
}

#define BUTTON_SETUP(pin)																	\
	gpio_dir(pin, GPIO_INPUT);																\
	pin_mode(pin, PIN_MODE_PULLUP);															\

	// pin_physport_from_pin(pin)->pcr[pin_physpin_from_pin(pin)].irqc = PCR_IRQC_INT_FALLING;

static void
buttons_init()
{
	BUTTON_SETUP(PIN_PTD0);
	BUTTON_SETUP(PIN_PTD1);
	BUTTON_SETUP(PIN_PTD2);
	BUTTON_SETUP(PIN_PTD3);
	BUTTON_SETUP(PIN_PTD4);
	BUTTON_SETUP(PIN_PTD5);
	BUTTON_SETUP(PIN_PTD6);
	BUTTON_SETUP(PIN_PTD7);
	BUTTON_SETUP(PIN_PTA1);
	BUTTON_SETUP(PIN_PTA2);
	BUTTON_SETUP(PIN_PTA4);
	BUTTON_SETUP(PIN_PTA18);
	BUTTON_SETUP(PIN_PTA19);
	BUTTON_SETUP(PIN_PTC4);
	BUTTON_SETUP(PIN_PTC5);
	BUTTON_SETUP(PIN_PTC6);
	BUTTON_SETUP(PIN_PTC7);
}

static void
hid_send_data_cb(void *buf, ssize_t len)
{
	buttons_poll();
	hid_send_data(&hid_ctx, &joystick_data, sizeof(struct joystick_data_t), JOYSTICK_TX_SIZE, hid_send_data_cb);
}

static void
init_my_hid_joy(int config) // see desc.h
{
	hid_init(&hid_funcs, &hid_ctx);
	buttons_init();
	hid_send_data_cb(NULL, 0);
}

void
main(void)
{
	usb_init(&hid_dev);
	sys_yield_for_frogs();
}
