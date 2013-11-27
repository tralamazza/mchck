#include <mchck.h>
#include <usb/usb.h>
#include <usb/cdc-acm.h>
#include "nRF24L01plus.h"

static struct cdc_ctx cdc;

static struct nrf_reg_config_t config = {
	.PRIM_RX = 0,
	.PWR_UP = 0,
	.CRCO = 0,
	.EN_CRC = 1,
	.MASK_RX_DR = 1,
	.MASK_TX_DS = 1,
	.MASK_MAX_RT = 1,
	.pad = 0
};

static void
print_data(void* data)
{
	// onboard_led(ONBOARD_LED_TOGGLE);
	printf("value 0x%x\r\n", *(uint8_t*)data);
	struct nrf_reg_config_t *cfg = data;
	printf("%u\r\n%u\r\n%u\r\n%u\r\n%u\r\n%u\r\n%u\r\n%u\r\n",
		cfg->PRIM_RX,
		cfg->PWR_UP,
		cfg->CRCO,
		cfg->EN_CRC,
		cfg->MASK_MAX_RT,
		cfg->MASK_MAX_RT,
		cfg->MASK_RX_DR,
		cfg->pad);
	// printf("cfg.value %u\r\n", *(uint8_t*)cfg);
}

static void
new_data(uint8_t *data, size_t len)
{
	onboard_led(ONBOARD_LED_TOGGLE);
	if (data[0] == 'w') {
		printf("write config reg %u\r\n", *(uint8_t*)&config);
		nrf_write_register(0x0, &config, 1, print_data);
	} else {
		uint8_t reg = data[0] - 97;
		printf("read 0x%x\r\n", reg);
		nrf_read_register(reg, print_data);
	}
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
