#include <mchck.h>
#include <usb/usb.h>
#include <usb/cdc-acm.h>
#include "nRF24L01plus.h"

#define CYCLES 10

static struct cdc_ctx cdc;
static struct timeout_ctx t;
static uint8_t channel = 0;
static uint8_t cycles = CYCLES;
static struct nrf_reg_config_t config = {
	.PRIM_RX = 1,
	.PWR_UP = 1,
	.CRCO = 0,
	.EN_CRC = 1,
	.MASK_RX_DR = 1,
	.MASK_TX_DS = 1,
	.MASK_MAX_RT = 1,
	.pad = 0
};

static uint32_t map[4] = { 0, 0, 0, 0 };

static void sched_rpd_read();

static void
rpd_data(void *data)
{
	uint8_t value = *(uint8_t*)data;
	uint8_t i = channel / 32;
	uint8_t b = channel % 32;
	map[i] |= (value << b);
	// pick the next ch
	if (++channel > 127) {
		channel = 0;
        if (--cycles == 0) {
	      onboard_led(ONBOARD_LED_TOGGLE);
          map[0] = 0;
          map[1] = 0;
          map[2] = 0;
          map[3] = 0;
          cycles = CYCLES;
        }
	}
	nrf_write_register(0x05, &channel, 1, sched_rpd_read);
}

static void
sched_rpd_read()
{
	nrf_read_register(0x09, rpd_data);
}

static void
put_ce_high()
{
	gpio_write(PIN_PTC3, 1);
	sched_rpd_read(); // start rpd
}

static void
delayed_init()
{
	// set RX mode and PWR_UP; RPD requires the nrf to be in RX MODE
	nrf_write_register(0x0, &config, 1, put_ce_high);
}

static void
new_data(uint8_t *data, size_t len)
{
	printf("%08lx %08lx %08lx %08lx\r\n", map[3], map[2], map[1], map[0]);
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
					u"nrf scanner",      /* product" */
					(init_vcdc,          /* init */
					 CDC)                /* functions */
	);

int
main(void)
{
	timeout_init();
	nrf_init();
	usb_init(&cdc_device);
	timeout_add(&t, 100, delayed_init, NULL); // wait 100ms before starting
	sys_yield_for_frogs();
}

