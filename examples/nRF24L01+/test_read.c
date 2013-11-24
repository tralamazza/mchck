#include <mchck.h>
#include <usb/usb.h>
#include <usb/cdc-acm.h>
#include "nRF24L01plus.h"

static struct cdc_ctx cdc;

static struct nrf_reg_config_t config = {
	.pad = 0,
	.MASK_RX_DR = 1,
	.MASK_TX_DS = 1,
	.MASK_MAX_RT = 1,
	.EN_CRC = 0
};

static void
print_data(void* data)
{
	onboard_led(ONBOARD_LED_TOGGLE);
	printf("value 0x%x\r\n", *(uint8_t*)data);
}

static void
new_data(uint8_t *data, size_t len)
{
	onboard_led(ONBOARD_LED_TOGGLE);
	uint8_t reg = data[0] - 97;
	printf("read 0x%x\r\n", reg);
	if (data[0] == 'w')
		nrf_write_register(0x0, &config, 1, print_data);
	else
		nrf_read_register(reg, print_data);
	cdc_read_more(&cdc);
}

static void
init_vcdc(int config)
{
	cdc_init(new_data, NULL, &cdc);
	cdc_set_stdout(&cdc);
}

static const struct usbd_device cdc_device =
	USB_INIT_DEVICE(0x2323,              /* vid */
					3,                   /* pid */
					u"mchck.org",        /* vendor */
					u"nrf test",         /* product" */
					(init_vcdc,          /* init */
					 CDC)                /* functions */
	);

void
main(void)
{
	timeout_init();
	nrf_init();
	usb_init(&cdc_device);
	sys_yield_for_frogs();
}
