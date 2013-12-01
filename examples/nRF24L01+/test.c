#include <mchck.h>
#include <usb/usb.h>
#include <usb/cdc-acm.h>
#include "nRF24L01plus.h"

#define RX_SIZE 4

// ping pong a uint32_t

static struct timeout_ctx t;
static int32_t tx_buffer = 0;
static uint8_t rx_buffer[RX_SIZE];

static struct nrf_addr_t target_addr = {
	.value =  0xf0f0f0f0d2ll,
	.size = 5
};
static struct nrf_addr_t my_addr = {
	.value =  0xf0f0f0f0e1ll,
	.size = 5
};

static void wait_for_ping();
static void send_ping();

static void
ping_sent(struct nrf_addr_t *receiver, void *data, uint8_t len)
{
	if (!receiver)
		onboard_led(ONBOARD_LED_TOGGLE);
	wait_for_ping();
}

static void
ping_received(struct nrf_addr_t *sender, void *data, uint8_t len)
{
	tx_buffer = *(int32_t*)data;
	send_ping();
}

static void
wait_for_ping()
{
	nrf_receive(&my_addr, rx_buffer, RX_SIZE, ping_received);
}

static void
send_ping()
{
	nrf_send(&target_addr, &tx_buffer, sizeof(int32_t), ping_sent);
}

static void
delayed_init(void *nada)
{
	wait_for_ping();
}

int
main(void)
{
	timeout_init();
	timeout_add(&t, 100, delayed_init, NULL);
	nrf_init();
	sys_yield_for_frogs();
}
