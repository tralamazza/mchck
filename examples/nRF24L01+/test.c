#include <mchck.h>
#include "nRF24L01plus.h"

#define RX_SIZE 32
#define CHANNEL 5

static struct timeout_ctx t;
static uint8_t rx_buffer[RX_SIZE];
static uint8_t tx_buffer = 0;

static struct nrf_addr_t test_addr = {
	.value =  { 0xaa, 0xde, 0xad, 0xbe, 0xef },
	.size = 5
};

static void
data_received(struct nrf_addr_t *sender, void *data, uint8_t len)
{
	onboard_led(*(uint8_t*)data);
}

static void
data_sent(struct nrf_addr_t *receiver, void *data, uint8_t len)
{
	// len == 0 when tx fifo full
	// len < 0 when max retransmit
	nrf_receive(rx_buffer, RX_SIZE, data_received);
}

static void
ping(void *data)
{
	tx_buffer ^= 1;
	nrf_send(&test_addr, &tx_buffer, 1, data_sent);
	timeout_add(&t, 200, ping, NULL);
}

int
main(void)
{
	nrf_init();
	nrf_set_channel(CHANNEL);
	nrf_set_rate_and_power(NRF_DATA_RATE_1MBPS, NRF_TX_POWER_0DBM);
	timeout_init();
	ping(NULL);
	sys_yield_for_frogs();
}
