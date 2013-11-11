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

enum {
	NRF_CE_TX = 0,
	NRF_CE_RX = 1
};

static nrf_data_available *data_available;
static nrf_data_sent *data_sent;

void
read_register(enum NRF_REG_ADDR reg, void *data, uint16_t len, spi_cb *cb)
{
	static uint8_t cmd = NRF_CMD_R_REGISTER;
	cmd |= (NRF_REG_MASK & reg);
	static struct spi_ctx r_register_ctx;
	spi_queue_xfer(&r_register_ctx, NRF_SPI_CS,
		&cmd, 1,
		data, len,
		cb, data);
}

void
write_register(enum NRF_REG_ADDR reg, uint8_t *data, uint16_t len, spi_cb *cb)
{
	static uint8_t cmd = NRF_CMD_W_REGISTER;
	cmd |= (NRF_REG_MASK & reg);
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
write_register_single(enum NRF_REG_ADDR reg, uint8_t data, spi_cb *cb)
{
	static uint8_t cmd = NRF_CMD_W_REGISTER;
	cmd |= (NRF_REG_MASK & reg);
	static struct spi_ctx_bare w_register_ctx;
	static struct sg tx_sg[2];
	static struct sg rx_sg;
	static uint8_t rx_buff; // only read the status
	spi_queue_xfer_sg(&w_register_ctx, NRF_SPI_CS,
			sg_init(tx_sg, &cmd, 1, &data, 1),
			sg_init(&rx_sg, &rx_buff, 1),
			cb, &rx_buff);
}

void
read_rx_payload(uint8_t *data, uint16_t len, spi_cb *cb)
{
	static uint8_t cmd = NRF_CMD_R_RX_PAYLOAD;
	static struct spi_ctx r_rx_payload_ctx;
	spi_queue_xfer(&r_rx_payload_ctx, NRF_SPI_CS,
		&cmd, 1,
		data, len,
		cb, data);
}

void
write_tx_payload(uint8_t *data, uint16_t len, spi_cb *cb)
{
	static uint8_t cmd = NRF_CMD_W_TX_PAYLOAD;
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
	static uint8_t cmd = NRF_CMD_FLUSH_TX;
	static struct spi_ctx flush_tx_ctx;
	spi_queue_xfer(&flush_tx_ctx, NRF_SPI_CS,
		&cmd, 1,
		NULL, 0,
		cb, NULL);
}

void
flush_rx(spi_cb *cb)
{
	static uint8_t cmd = NRF_CMD_FLUSH_RX;
	static struct spi_ctx flush_rx_ctx;
	spi_queue_xfer(&flush_rx_ctx, NRF_SPI_CS,
		&cmd, 1,
		NULL, 0,
		cb, NULL);
}

void
reuse_tx_pl(spi_cb *cb)
{
	static uint8_t cmd = NRF_CMD_REUSE_TX_PL;
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
	if (ctx->rxbuf[1] > 32) {
		flush_rx(NULL); // flush RX if packet width > 32
	}
	ctx->cb(&ctx->rxbuf[1]);
}

void
read_rx_payload_width(spi_cb *cb)
{
	static uint8_t cmd = NRF_CMD_R_RX_PL_WID;
	static struct spi_ctx read_rx_payload_ctx;
	static struct read_rx_payload_width_t ctx;
	ctx.cb = cb;
	spi_queue_xfer(&read_rx_payload_ctx, NRF_SPI_CS,
		&cmd, 1,
		ctx.rxbuf, 2,
		read_rx_payload_width_cb, &ctx);
}

void
write_ack_payload(uint8_t pipe, uint8_t *data, uint16_t len, spi_cb *cb)
{
	static uint8_t cmd = NRF_CMD_W_ACK_PAYLOAD;
	cmd |= (pipe & 0x7);
	static struct spi_ctx_bare w_ack_payload_ctx;
	static struct sg tx_sg[2];
	spi_queue_xfer_sg(&w_ack_payload_ctx, NRF_SPI_CS,
		sg_init(tx_sg, &cmd, 1, data, len),
		NULL,
		cb, NULL);
}

void
write_tx_payload_no_ack(uint8_t *data, uint16_t len, spi_cb *cb)
{
	static uint8_t cmd = NRF_CMD_W_TX_PAYLOAD_NO_ACK;
	static struct spi_ctx_bare w_tx_payload_ctx;
	static struct sg tx_sg[2];
	spi_queue_xfer_sg(&w_tx_payload_ctx, NRF_SPI_CS,
		sg_init(tx_sg, &cmd, 1, data, len),
		NULL,
		cb, NULL);
}

void
nop(spi_cb *cb)
{
	static uint8_t cmd = NRF_CMD_NOP;
	static struct spi_ctx nop_ctx;
	static uint8_t rx_buff;
	spi_queue_xfer(&nop_ctx, NRF_SPI_CS,
		&cmd, 1,
		&rx_buff, 1,
		cb, &rx_buff);
}

void
PORTC_Handler__read_rx_payload(void *data)
{

}

void
PORTC_Handler__status(void *data)
{
	struct status_register_t *status = data;
	if (status->RX_DR) {
		read_rx_payload( PORTC_Handler__read_rx_payload);
	}

	if (status->TX_DS) {
	}

	if (status->MAX_RT) {
	}
}

void
PORTC_Handler(void)
{
	nop(PORTC_Handler__status);
}

void
nrf_init(nrf_data_available *cb_avail, nrf_data_sent *cb_sent)
{
	data_available = cb_avail;
	data_sent = cb_sent;

	gpio_dir(NRF_CE, GPIO_OUTPUT);
	gpio_write(NRF_CE, NRF_CE_RX);

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
	nrf_init(NULL, NULL);
	sys_yield_for_frogs();
}
