#include <mchck.h>
#include "nrf905.h"
#include "nrf905_usb.desc.h"

#define ADDR_LEN 4

static struct cdc_ctx cdc;
static struct nrf905_ctx_t ctx;
static uint8_t my_addr[ADDR_LEN] = { 0xaa, 0xaa, 0xaa, 0xaa };
static uint8_t target_addr[ADDR_LEN] = { 0xbb, 0xbb, 0xbb, 0xbb };
static uint8_t payload[32];
static uint8_t payload_len = 32;

static enum {
	APP_STATE_INIT,
	APP_STATE_ADDR_SET,
	APP_STATE_DONE,
} app_state = APP_STATE_INIT;

static void
nrf905_recv_done(void *data, uint8_t len)
{
	onboard_led(ONBOARD_LED_TOGGLE);
	cdc_write(data, len, &cdc);
}

static void
nrf905_app_state_handler(void *data)
{
	switch (app_state) {
	case APP_STATE_INIT:
		onboard_led(ONBOARD_LED_TOGGLE);
		app_state = APP_STATE_ADDR_SET;
		nrf905_set_rx_addr(&ctx, my_addr, ADDR_LEN, nrf905_app_state_handler);
		break;
	case APP_STATE_ADDR_SET:
		onboard_led(ONBOARD_LED_TOGGLE);
		app_state = APP_STATE_DONE;
		nrf905_set_tx_addr(&ctx, target_addr, ADDR_LEN, nrf905_app_state_handler);
	case APP_STATE_DONE:
		nrf905_receive(&ctx, payload, payload_len, nrf905_recv_done);
		break;
	}
}

void
cdc_data_sent(size_t len)
{
	nrf905_receive(&ctx, payload, payload_len, nrf905_recv_done);
}

void
init_vcdc(int enable)
{
	if (enable) {
		cdc_init(NULL, cdc_data_sent, &cdc);
	}
}

NRF905_INIT_DECL(&ctx);

int
main(void)
{
	usb_init(&cdc_device);
	nrf905_init(&ctx, nrf905_app_state_handler);
	sys_yield_for_frogs();
}