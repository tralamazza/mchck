#include <mchck.h>
#include "nrf905.h"
#include "nrf905_usb.desc.h"

#define ADDR_LEN 4
#define PAYLOAD_LEN 32

static struct cdc_ctx cdc;
static struct nrf905_ctx_t ctx;
static uint8_t my_addr[ADDR_LEN] = { 0xaa, 0xaa, 0xaa, 0xaa };
static uint8_t target_addr[ADDR_LEN] = { 0xbb, 0xbb, 0xbb, 0xbb };
static uint8_t payload[PAYLOAD_LEN];

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
nrf905_send_done(void *data, uint8_t len)
{
	cdc_read_more(&cdc);
	nrf905_receive(&ctx, payload, PAYLOAD_LEN, nrf905_recv_done);
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
		nrf905_receive(&ctx, payload, PAYLOAD_LEN, nrf905_recv_done);
		break;
	}
}

static void
cdc_data_sent(size_t len)
{
}

static void
cdc_new_data(uint8_t *data, size_t len)
{
	nrf905_send(&ctx, data, len, nrf905_send_done);
}

void
init_vcdc(int enable)
{
	if (enable) {
		cdc_init(cdc_new_data, cdc_data_sent, &cdc);
	}
}

NRF905_INT_DECL(&ctx);

int
main(void)
{
	spi_init();
	pin_change_init();
	usb_init(&cdc_device);
	nrf905_init(&ctx, nrf905_app_state_handler);
	sys_yield_for_frogs();
}