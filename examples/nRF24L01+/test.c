#include <mchck.h>
#include <usb/usb.h>
#include <usb/cdc-acm.h>
#include "nRF24L01plus.h"

// static struct cdc_ctx cdc;
#define RX_SIZE 4

static struct timeout_ctx t;
static int32_t tx_buffer = 0;
static uint8_t rx_buffer[RX_SIZE];

static struct nrf_addr_t target_addr = {
	// .value =  { 'c', 'l', 'i', 'e', '1' },
	// .value =  { '1', 'e', 'i', 'l', 'c' },

	// .value =  { 0xe9, 0xe9, 0xf1, 0xf1, 0xe2 },
	// .value =  { 0xe2, 0xf1, 0xf1, 0xe9, 0xe9 },

	.value =  0xf0f0f0f0d2ll,
	// .value =  0xd2f0f0f0f0ll,
	// .value =  { 0xf0, 0xf0, 0xf0, 0xf0, 0xd2 },
	// .value =  { 0xd2, 0xf0, 0xf0, 0xf0, 0xf0 },

	// .value =  { 0xf0, 0xf0, 0xf0, 0xf0, 0xe1 },
	// .value =  { 0xe1, 0xf0, 0xf0, 0xf0, 0xf0 },

	.size = 5
};
static struct nrf_addr_t my_addr = {
	// .value =  { 's', 'e', 'r', 'v', '1' },
	// .value =  { '1', 'v', 'r', 'e', 's' },

	// .value =  { 0xe8, 0xe8, 0xf0, 0xf0, 0xe1 },
	// .value =  { 0xe1, 0xf0, 0xf0, 0xe8, 0xe8 },

	.value =  0xf0f0f0f0e1ll,
	// .value =  0xe1f0f0f0f0ll,
	// .value =  { 0xf0, 0xf0, 0xf0, 0xf0, 0xe1 },
	// .value =  { 0xe1, 0xf0, 0xf0, 0xf0, 0xf0 },

	// .value =  { 0xf0, 0xf0, 0xf0, 0xf0, 0xd2 },
	// .value =  { 0xd2, 0xf0, 0xf0, 0xf0, 0xf0 },

	.size = 5
};

static void wait_for_ping();
static void send_ping();

static void
ping_sent(struct nrf_addr_t *receiver, void *data, uint8_t len)
{
	// onboard_led(ONBOARD_LED_TOGGLE);
	wait_for_ping();
	// timeout_add(&t, 50, send_ping, NULL);
}

static void
ping_received(struct nrf_addr_t *sender, void *data, uint8_t len)
{
	// onboard_led(ONBOARD_LED_TOGGLE);
	// tx_buffer++;
	// nrf_send(&target_addr, &tx_buffer, sizeof(unsigned), ping_sent);
	// wait_for_ping();

	timeout_add(&t, 50, send_ping, NULL);
}

static void
wait_for_ping()
{
	nrf_receive(&my_addr, rx_buffer, RX_SIZE, ping_received);
}

static void
send_ping()
{
	tx_buffer++;
	nrf_send(&target_addr, &tx_buffer, sizeof(int32_t), ping_sent);
}

// static void
// new_data(uint8_t *data, size_t len)
// {
// 	onboard_led(ONBOARD_LED_TOGGLE);
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
delayed_init(void *nada)
{
	wait_for_ping();
}

int
main(void)
{
	timeout_init();
	timeout_add(&t, 200, delayed_init, NULL);
	// usb_init(&cdc_device);
	nrf_init();
	sys_yield_for_frogs();
}
