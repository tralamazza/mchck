#include <mchck.h>
#include <nrf/nrf.h>

// Wait for (at least) a uint32_t and send it back

#define RX_SIZE 32

#define RX_ADDR 0xf0f0f0f0e1ll
#define TX_ADDR 0xf0f0f0f0d2ll

static int32_t tx_buffer = 0;
static uint8_t rx_buffer[RX_SIZE];

static struct nrf_addr_t target_addr = { .value =  TX_ADDR, .size = 5 };
static struct nrf_addr_t my_addr = { .value =  RX_ADDR, .size = 5 };

static void wait_for_ping();
static void send_ping();

static void
ping_sent(void *data, uint8_t len)
{
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
	onboard_led(ONBOARD_LED_TOGGLE);
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
	nrf_set_autoretransmit(3, 5); // 1000us, 5x
	nrf_set_crc_length(NRF_CRC_ENC_2_BYTES); // CRC16
	nrf_set_channel(76);
	nrf_set_power_datarate(NRF_TX_POWER_0DBM, NRF_DATA_RATE_1MBPS);
	nrf_enable_dynamic_payload();
	// nrf_enable_powersave();
	send_ping();
	sys_yield_for_frogs();
}
