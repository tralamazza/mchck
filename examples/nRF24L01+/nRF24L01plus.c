#include <mchck.h>
#include "nRF24L01plus.h"

/* pin mapping */
enum {
	NRF_CE   = PIN_PTC3;
	NRF_CSN  = PIN_PTC2;
	NRF_SCK  = PIN_PTC5;
	NRF_MOSI = PIN_PTC6;
	NRF_MISO = PIN_PTC7;
	NRF_IRQ  = PIN_PTC4;
};

void
read_register(enum NRF_REG_ADDR reg, uint8_t *value, uint16_t len)
{
	struct spi_ctx r_register_ctx;
	uint8_t cmd = NRF_CMD_R_REGISTER | (NRF_REG_MASK & reg);
	spi_queue_xfer(&r_register_ctx, NRF_SPI_CS,
		&cmd, 1,
		value, len,
		NULL, NULL);
}

void
write_register(enum NRF_REG_ADDR reg, uint8_t *value, uint16_t len)
{
	struct spi_ctx w_register_ctx;
	uint8_t cmd = NRF_CMD_W_REGISTER | (NRF_REG_MASK & reg);
	spi_queue_xfer(&w_register_ctx, NRF_SPI_CS,
		&cmd, 1,
		NULL, 0,
		NULL, NULL);
	spi_queue_xfer(&w_register_ctx, NRF_SPI_CS,
		value, len,
		NULL, 0,
		NULL, NULL);
}

void
read_rx_payload(uint8_t *value, uint16_t len)
{
	struct spi_ctx r_rx_payload_ctx;
	uint8_t cmd = NRF_CMD_R_RX_PAYLOAD;
	spi_queue_xfer(&r_rx_payload_ctx, NRF_SPI_CS,
		&cmd, 1,
		value, len,
		NULL, NULL);
}

void
write_tx_payload(uint8_t *value, uint16_t len)
{
	struct spi_ctx w_tx_payload_ctx;
	uint8_t cmd = NRF_CMD_W_TX_PAYLOAD;
	spi_queue_xfer(&w_tx_payload_ctx, NRF_SPI_CS,
		&cmd, 1,
		NULL, 0,
		NULL, NULL);
	spi_queue_xfer(&w_tx_payload_ctx, NRF_SPI_CS,
		value, len,
		NULL, 0,
		NULL, NULL);
}

void
flush_tx(void)
{
	struct spi_ctx flush_tx_ctx;
	uint8_t cmd = NRF_CMD_FLUSH_TX;
	spi_queue_xfer(&flush_tx_ctx, NRF_SPI_CS,
		&cmd, 1,
		NULL, 0,
		NULL, NULL);
}

void
flush_rx(void)
{
	struct spi_ctx flush_rx_ctx;
	uint8_t cmd = NRF_CMD_FLUSH_RX;
	spi_queue_xfer(&flush_rx_ctx, NRF_SPI_CS,
		&cmd, 1,
		NULL, 0,
		NULL, NULL);
}

void
reuse_tx_pl(void)
{
	struct spi_ctx reuse_tx_ctx;
	uint8_t cmd = NRF_CMD_REUSE_TX_PL;
	spi_queue_xfer(&reuse_tx_ctx, NRF_SPI_CS,
		&cmd, 1,
		NULL, 0,
		NULL, NULL);
}

uint8_t
read_rx_payload_width(void)
{
	struct spi_ctx read_rx_payload_ctx;
	uint8_t rxbuf[2];
	uint8_t cmd = NRF_CMD_R_RX_PL_WID;
	spi_queue_xfer(&read_rx_payload_ctx, NRF_SPI_CS,
		&cmd, 1,
		rxbuf, 2,
		NULL, NULL);
	return rxbuf[1];
}

void
write_ack_payload(uint8_t *value, uint16_t len)
{
	struct spi_ctx w_ack_payload_ctx;
	uint8_t cmd = NRF_CMD_W_ACK_PAYLOAD;
	spi_queue_xfer(&w_ack_payload_ctx, NRF_SPI_CS,
		&cmd, 1,
		NULL, 0,
		NULL, NULL);
	spi_queue_xfer(&w_ack_payload_ctx, NRF_SPI_CS,
		value, len,
		NULL, 0,
		NULL, NULL);
}

void
write_tx_payload_no_ack()
{
	static struct spi_ctx w_tx_payload_ctx;
	uint8_t cmd = NRF_CMD_W_TX_PAYLOAD;
	spi_queue_xfer(&w_tx_payload_ctx, NRF_SPI_CS,
		&cmd, 1,
		NULL, 0,
		NULL, NULL);
	spi_queue_xfer(&w_tx_payload_ctx, NRF_SPI_CS,
		value, len,
		NULL, 0,
		NULL, NULL);
}

void
nop(void)
{
	static struct spi_ctx nop_ctx;
	uint8_t cmd = NRF_CMD_NOP;
	spi_queue_xfer(&nop_ctx, NRF_SPI_CS,
		&cmd, 1,
		NULL, 0,
		NULL, NULL);
}

struct nrf_disable_shockburst_t {

};

// #######
void
disable_shockburst()
{
	read_register(NRF_REG_ADDR_RF_SETUP);
	write_status_register(uint8_t reg, uint8_t *value, uint16_t len)
}


void
set_auto_ack(int enabled, uint8_t pipe)
{

}
// #######


void
PORTC_Handler(void)
{

}

void
nrf_init(void)
{
	gpio_dir(NRF_CE, GPIO_OUTPUT);
	gpio_write(NRF_CE, 1);

	pin_mode(NRF_CSN, PIN_MODE_MUX_ALT2);
	pin_mode(NRF_SCK, PIN_MODE_MUX_ALT2);
	pin_mode(NRF_MOSI, PIN_MODE_MUX_ALT2);
	pin_mode(NRF_MISO, PIN_MODE_MUX_ALT2);

	// nrf interrupt
	pin_mode(NRF_IRQ, PIN_MODE_PULLDOWN);
	pin_physport_from_pin(NRF_IRQ)->pcr[pin_physpin_from_pin(NRF_IRQ)].irqc = PCR_IRQC_INT_RISING;
	int_enable(IRQ_PORTC);
}

int
main(void)
{
	spi_init();
	nrf_init();
	sys_yield_for_frogs();
}
