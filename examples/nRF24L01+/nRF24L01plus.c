#include <mchck.h>
#include "nRF24L01plus.h"

/* pin mapping */
enum {
	NRF_CE   = PIN_PTC3,
	NRF_CSN  = PIN_PTC2,
	NRF_SCK  = PIN_PTC5,
	NRF_MOSI = PIN_PTC6,
	NRF_MISO = PIN_PTC7,
	NRF_IRQ  = PIN_PTC4
};

void
read_register(enum NRF_REG_ADDR reg, void *data, uint16_t len, spi_cb *cb)
{
	uint8_t cmd = NRF_CMD_R_REGISTER | (NRF_REG_MASK & reg);
	static struct spi_ctx r_register_ctx;
	spi_queue_xfer(&r_register_ctx, NRF_SPI_CS,
		&cmd, 1,
		data, len,
		cb, data);
}

void
write_register(enum NRF_REG_ADDR reg, uint8_t *data, uint16_t len, spi_cb *cb)
{
	uint8_t cmd = NRF_CMD_W_REGISTER | (NRF_REG_MASK & reg);
	static struct spi_ctx_bare w_register_ctx;
	static struct sg tx_sg[2];
	static struct sg rx_sg;
	static uint8_t rx_buff; // only read the status
	spi_queue_xfer_sg(&w_register_ctx, NRF_SPI_CS,
			sg_init(tx_sg, &cmd, 1, data, len),
			sg_init(&rx_sg, &rx_buff, 1),
			cb, &rx_buff);
}

void
read_rx_payload(uint8_t *data, uint16_t len, spi_cb *cb)
{
	static struct spi_ctx r_rx_payload_ctx;
	uint8_t cmd = NRF_CMD_R_RX_PAYLOAD;
	spi_queue_xfer(&r_rx_payload_ctx, NRF_SPI_CS,
		&cmd, 1,
		data, len,
		cb, data);
}

void
write_tx_payload(uint8_t *data, uint16_t len, spi_cb *cb)
{
	uint8_t cmd = NRF_CMD_W_TX_PAYLOAD;
	static struct spi_ctx_bare w_tx_payload_ctx;
	static struct sg tx_sg[2];
	spi_queue_xfer_sg(&w_tx_payload_ctx, NRF_SPI_CS,
		sg_init(tx_sg, &cmd, 1, data, len),
		NULL,
		cb, NULL);
}

void
flush_tx(spi_cb *cb)
{
	uint8_t cmd = NRF_CMD_FLUSH_TX;
	static struct spi_ctx flush_tx_ctx;
	spi_queue_xfer(&flush_tx_ctx, NRF_SPI_CS,
		&cmd, 1,
		NULL, 0,
		cb, NULL);
}

void
flush_rx(spi_cb *cb)
{
	uint8_t cmd = NRF_CMD_FLUSH_RX;
	static struct spi_ctx flush_rx_ctx;
	spi_queue_xfer(&flush_rx_ctx, NRF_SPI_CS,
		&cmd, 1,
		NULL, 0,
		cb, NULL);
}

void
reuse_tx_pl(spi_cb *cb)
{
	uint8_t cmd = NRF_CMD_REUSE_TX_PL;
	static struct spi_ctx reuse_tx_ctx;
	spi_queue_xfer(&reuse_tx_ctx, NRF_SPI_CS,
		&cmd, 1,
		NULL, 0,
		cb, NULL);
}

struct read_rx_payload_width_t {
	uint8_t rxbuf[2];
	spi_cb *cb;
};

void
read_rx_payload_width_cb(void *data)
{
	struct read_rx_payload_width_t *ctx = data;
	ctx->cb(&ctx->rxbuf[1]); // flush RX if packet width > 32
}

void
read_rx_payload_width(spi_cb *cb)
{
	static struct spi_ctx read_rx_payload_ctx;
	static struct read_rx_payload_width_t ctx;
	static uint8_t cmd = NRF_CMD_R_RX_PL_WID;
	ctx.cb = cb;
	spi_queue_xfer(&read_rx_payload_ctx, NRF_SPI_CS,
		&cmd, 1,
		ctx.rxbuf, 2,
		read_rx_payload_width_cb, &ctx);
}
/*
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
}*/

void
nop(void)
{
	static struct spi_ctx nop_ctx;
	static uint8_t cmd = NRF_CMD_NOP;
	spi_queue_xfer(&nop_ctx, NRF_SPI_CS,
		&cmd, 1,
		NULL, 0,
		NULL, NULL);
}

struct nrf_disable_shockburst_t {

};

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
