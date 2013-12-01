#include <mchck.h>
#include <usb/usb.h>
#include <usb/cdc-acm.h>
#include "nRF24L01plus.h"

// Wait for a uint32_t and send it back

#define RX_SIZE 4

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
ping_sent(void *data, uint8_t len)
{
	if (!len)
		onboard_led(ONBOARD_LED_TOGGLE); // send failed, max RT reached
	wait_for_ping();
}

static void
send_ping()
{
	nrf_send(&target_addr, &tx_buffer, sizeof(int32_t), ping_sent);
}

static void
ping_received(void *data, uint8_t len)
{
	tx_buffer = *(int32_t*)data;
	send_ping();
}

static void
wait_for_ping()
{
	nrf_receive(&my_addr, rx_buffer, RX_SIZE, ping_received);
}

int
main(void)
{
	nrf_init();
	nrf_set_channel(16);
	nrf_set_power_datarate(NRF_TX_POWER_0DBM, NRF_DATA_RATE_1MBPS);
	wait_for_ping();
	sys_yield_for_frogs();
}
