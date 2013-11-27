#include <mchck.h>
#include "nrf.h"

#define NRF_REG_MASK 0x1f // 0001 1111

/* Pin mapping */
enum {
	NRF_CE   = PIN_PTC3,
	NRF_CSN  = PIN_PTC2,
	NRF_SCK  = PIN_PTC5,
	NRF_MOSI = PIN_PTC6,
	NRF_MISO = PIN_PTC7,
	NRF_IRQ  = PIN_PTC4
};

/* SPI */
enum {
	NRF_SPI_CS = SPI_PCS2
};

/* SPI transaction */
struct nrf_transaction_t {
	struct spi_ctx_bare spi_ctx;
	struct sg tx_sg[2];
	struct sg rx_sg[2];
	uint8_t cmd;
	struct nrf_status_t status;
	uint8_t tx_len;
	uint8_t rx_len;
	void *tx_data;
	void *rx_data;
};

inline static void
send_command(struct nrf_transaction_t *trans, spi_cb cb, void *data)
{
	struct sg *tx = trans->tx_len ?
		sg_init(trans->tx_sg, &trans->cmd, 1, trans->tx_data, trans->tx_len) :
		sg_init(trans->tx_sg, &trans->cmd, 1);
	struct sg *rx = trans->rx_len ?
		sg_init(trans->rx_sg, &trans->status, 1, trans->rx_data, trans->rx_len) :
		sg_init(trans->rx_sg, &trans->status, 1);

	spi_queue_xfer_sg(&trans->spi_ctx, NRF_SPI_CS, tx, rx, cb, data);
}

static void
notify_callback(void *data)
{
	*(int*)data = 1;
}

#define BLOCK_SEND_COMMAND(_ptrans)							\
	volatile int sem = 0;									\
	send_command(_ptrans, notify_callback, (void*)&sem);	\
	while (!sem)											\
		__asm__("wfi");										\

static void
nrf_int_handler(void *data)
{
	// struct nrf_transaction_t *int_trans = data;
}

void
PORTC_Handler(void)
{
	pin_physport_from_pin(NRF_IRQ)->pcr[pin_physpin_from_pin(NRF_IRQ)].raw |= 0; // clear MCU interrupt
	static struct nrf_transaction_t trans = {
		.cmd = NRF_CMD_NOP,
		.tx_len = 0,
		.rx_len = 0
	};
	send_command(&trans, nrf_int_handler, &trans);
}

void
nrf_init(void)
{
	spi_init();

	pin_mode(NRF_CSN, PIN_MODE_MUX_ALT2);
	pin_mode(NRF_SCK, PIN_MODE_MUX_ALT2);
	pin_mode(NRF_MOSI, PIN_MODE_MUX_ALT2);
	pin_mode(NRF_MISO, PIN_MODE_MUX_ALT2);

	gpio_dir(NRF_CE, GPIO_OUTPUT);
	gpio_write(NRF_CE, 0);

	gpio_dir(NRF_IRQ, GPIO_INPUT);
	pin_physport_from_pin(NRF_IRQ)->pcr[pin_physpin_from_pin(NRF_IRQ)].irqc = PCR_IRQC_INT_FALLING;
	int_enable(IRQ_PORTC);

	// defaults
	nrf_set_channel(42);
	nrf_set_power_rate(NRF_TX_POWER_0DBM, NRF_DATA_RATE_1MBPS);
}

void
nrf_read_register(uint8_t reg, void *data, size_t len)
{
	struct nrf_transaction_t trans = {
		.cmd = NRF_CMD_R_REGISTER | (NRF_REG_MASK & reg),
		.tx_len = 0,
		.tx_data = NULL,
		.rx_len = len,
		.rx_data = data
	};
	BLOCK_SEND_COMMAND(&trans);
}

uint8_t
nrf_read_register_byte(uint8_t reg)
{
	uint8_t result = 0;
	nrf_read_register(reg, &result, 1);
	return result;
}

void
nrf_write_register(uint8_t reg, void *data, size_t len)
{
	struct nrf_transaction_t trans = {
		.cmd = NRF_CMD_W_REGISTER | (NRF_REG_MASK & reg),
		.tx_len = len,
		.tx_data = data,
		.rx_len = 0,
		.rx_data = NULL
	};
	BLOCK_SEND_COMMAND(&trans);
}

void
nrf_write_register_byte(uint8_t reg, uint8_t value)
{
	nrf_write_register(reg, &value, 1);
}

void
nrf_set_channel(uint8_t channel)
{
	nrf_write_register_byte(NRF_REG_ADDR_RF_CH, channel & 0x7f);
}

uint8_t
nrf_get_channel()
{
	return nrf_read_register_byte(NRF_REG_ADDR_RF_CH) & 0x7f;
}

void
nrf_set_power_rate(enum nrf_tx_output_power_t power, enum nrf_data_rate_t data_rate)
{
	struct nrf_rf_setup_t rf_setup = {
		.pad2 = 0,
		.power = power,
		.rate = data_rate,
		.pad1 = 0,
		.CONT_WAVE = 0
	};
	nrf_write_register_byte(NRF_REG_ADDR_RF_SETUP, *(uint8_t*)&rf_setup);
}

void
nrf_set_payload_size(uint8_t pipe, uint8_t payload_size)
{
	struct nrf_datapipe_payload_size_t dps = {
		.size = payload_size,
		.pad = 0
	};
	nrf_write_register_byte(NRF_REG_ADDR_RX_PW_P0 + pipe, *(uint8_t*)&dps);
}

uint8_t
nrf_get_payload_size(uint8_t pipe)
{
	uint8_t value = nrf_read_register_byte(NRF_REG_ADDR_RX_PW_P0 + pipe);
	struct nrf_datapipe_payload_size_t *dps = (struct nrf_datapipe_payload_size_t*)&value;
	return dps->size;
}

void
nrf_set_rx_address(uint8_t pipe, uint64_t address, enum nrf_rxtx_addr_field_width address_size)
{
	nrf_write_register(NRF_REG_ADDR_RX_ADDR_P0 + pipe, (void*)&address, address_size);
}

void
nrf_set_tx_address(uint64_t address, enum nrf_rxtx_addr_field_width address_size)
{
	nrf_write_register(NRF_REG_ADDR_TX_ADDR, (void*)&address, address_size);
}

void
nrf_enable_auto_ack(uint8_t pipe)
{
	nrf_write_register_byte(NRF_REG_ADDR_EN_AA, 1 << pipe);
}

uint8_t
nrf_auto_ack_enabled(uint8_t pipe)
{
	return (1 << pipe) & nrf_read_register_byte(NRF_REG_ADDR_EN_AA);
}

void
nrf_enable_rx_address(uint8_t pipe)
{
	nrf_write_register_byte(NRF_REG_ADDR_EN_RXADDR, 1 << pipe);
}

uint8_t
nrf_rx_address_enabled(uint8_t pipe)
{
	return (1 << pipe) & nrf_read_register_byte(NRF_REG_ADDR_EN_RXADDR);
}

void
nrf_set_address_width(enum nrf_rxtx_addr_field_width width)
{
	nrf_write_register_byte(NRF_REG_ADDR_SETUP_AW, width & 0x3);
}

uint8_t
nrf_get_address_width()
{
	return nrf_read_register_byte(NRF_REG_ADDR_SETUP_AW) & 0x3;
}

void
nrf_set_auto_retransmit(uint8_t delay, uint8_t count)
{
	nrf_write_register_byte(NRF_REG_ADDR_SETUP_RETR, (delay & 0xf0) | (count & 0x0f));
}

void
nrf_enable_dynamic_payload(uint8_t pipe)
{
	nrf_write_register_byte(NRF_REG_ADDR_DYNPD, 1 << pipe);
}

uint8_t
nrf_dynamic_payload_enabled(uint8_t pipe)
{
	return (1 << pipe) & nrf_read_register_byte(NRF_REG_ADDR_DYNPD);
}
