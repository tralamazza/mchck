#include "nrf905.h"

enum NRF905_CMD {
	W_CONFIG	= 0x00,	// 0000 AAAA
	R_CONFIG	= 0x10,	// 0001 AAAA
	W_TX_PAYLOAD	= 0x20, // 0010 0000
	R_TX_PAYLOAD	= 0x21, // 0010 0001
	W_TX_ADDRESS	= 0x22, // 0010 0010
	R_TX_ADDRESS	= 0x23, // 0010 0011
	R_RX_PAYLOAD	= 0x24, // 0010 0100
	CHANNEL_CONFIG	= 0x8000 // 1000 pphc cccc cccc
};

enum {
	NRF905_SPI_CS = SPI_PCS2
};

#define CHANNEL_CONFIG_MASK 0x8f
#define CONFIG_REG_SIZE 10

static void
nrf905_send_command(struct nrf905_ctx_t *ctx, spi_cb *cb, void *data)
{
	struct sg *tx = ctx->trans.tx_len ?
		sg_init(ctx->trans.tx_sg, ctx->trans.cmd, ctx->trans.cmd_len, ctx->trans.tx_data, ctx->trans.tx_len) :
		sg_init(ctx->trans.tx_sg, ctx->trans.cmd, ctx->trans.cmd_len);
	struct sg *rx = ctx->trans.rx_len ?
		sg_init(ctx->trans.rx_sg, &ctx->status, 1, ctx->trans.rx_data, ctx->trans.rx_len) :
		sg_init(ctx->trans.rx_sg, &ctx->status, 1);
	spi_queue_xfer_sg(&ctx->trans.spi_ctx, NRF905_SPI_CS, tx, rx, cb, data);
}

static void
nrf905_read_config(struct nrf905_ctx_t *ctx, uint8_t offset, spi_cb *cb, void *data)
{
	ctx->trans.cmd[0] = R_CONFIG | offset;
	ctx->trans.cmd_len = 1;
	ctx->trans.tx_data = NULL;
	ctx->trans.tx_len = 0;
	ctx->trans.rx_data = &ctx->config;
	ctx->trans.rx_data += offset;
	ctx->trans.rx_len = CONFIG_REG_SIZE - offset; // TODO check this
	nrf905_send_command(ctx, cb, data);
}

static void
nrf905_write_config(struct nrf905_ctx_t *ctx, uint8_t offset, spi_cb *cb, void *data)
{
	ctx->trans.cmd[0] = W_CONFIG | offset;
	ctx->trans.cmd_len = 1;
	ctx->trans.tx_data = &ctx->config;
	ctx->trans.tx_data += offset;
	ctx->trans.tx_len = CONFIG_REG_SIZE - offset; // TODO check this
	ctx->trans.rx_data = NULL;
	ctx->trans.rx_len = 0;
	nrf905_send_command(ctx, cb, data);
}

static void
nrf905_handle_state(void *data)
{
	struct nrf905_ctx_t *ctx = data;
	switch (ctx->state) {
	case NRF905_IDLE:
		break;
	case NRF905_TX_LEN:
		ctx->trans.cmd[0] = W_TX_PAYLOAD;
		ctx->trans.cmd_len = 1;
		ctx->trans.tx_data = ctx->cb_data;
		ctx->trans.tx_len = ctx->cb_data_len;
		ctx->trans.rx_data = NULL;
		ctx->trans.rx_len = 0;
		ctx->state = NRF905_TX_PAYLOAD;
		nrf905_send_command(ctx, nrf905_handle_state, ctx);
		break;
	case NRF905_TX_PAYLOAD:
		ctx->state = NRF905_TX_DONE;
		gpio_write(NRF905_TRX_CE, 1);
		break;
	case NRF905_TX_DONE:
		ctx->state = NRF905_IDLE;
		gpio_write(NRF905_TRX_CE, 0);
		if (ctx->cb) {
			ctx->cb(ctx->cb_data, ctx->cb_data_len);
		}
		break;
	case NRF905_RX_LEN:
		ctx->state = NRF905_RX_PAYLOAD;
		gpio_write(NRF905_TRX_CE, 1);
		gpio_write(NRF905_TX_EN, 0);
		break;
	case NRF905_RX_PAYLOAD:
		gpio_write(NRF905_TRX_CE, 1);
		ctx->trans.cmd[0] = R_RX_PAYLOAD;
		ctx->trans.cmd_len = 1;
		ctx->trans.tx_data = NULL;
		ctx->trans.tx_len = 0;
		ctx->trans.rx_data = ctx->cb_data;
		ctx->trans.rx_len = ctx->cb_data_len;
		ctx->state = NRF905_RX_DONE;
		nrf905_send_command(ctx, NULL, NULL);
		break;
	case NRF905_RX_DONE:
		ctx->state = NRF905_IDLE;
		if (ctx->cb) {
			ctx->cb(ctx->cb_data, ctx->cb_data_len);
		}
		break;
	}
}

void
nrf905_reset(struct nrf905_ctx_t *ctx, spi_cb *cb, void *data)
{
	ctx->state = NRF905_IDLE;
	gpio_write(NRF905_PWR_UP, 0);

	ctx->config.CH_NO = 108; // default
	ctx->config.HFREQ_PLL = HFREQ_PLL_433MHZ; // default
	ctx->config.PA_PWR = PA_PWR_10DBM; // +10dBm
	ctx->config.RX_RED_PWR = 0; // default
	ctx->config.AUTO_RETRAN = 0; // default
	ctx->config.RX_AFW = ADDR_LEN_4BYTES; // default
	ctx->config.TX_AFW = ADDR_LEN_4BYTES; // default
	ctx->config.RX_PW = 32; // default
	ctx->config.TX_PW = 32; // default
	for (int i = 0; i < 4; i++) {
		ctx->config.RX_ADDRESS[i] = 0xe7; // default
	}
	ctx->config.UP_CLK_FREQ = UP_CLK_FREQ_500KHZ; // default
	ctx->config.UP_CLK_EN = 0; // no output clock
	ctx->config.XOF = XOF_16MHZ; // 16mhz osci
	ctx->config.CRC_EN = 1; // default
	ctx->config.CRC_MODE = CRC_MODE_16BITS; // default

	nrf905_write_config(ctx, 0, NULL, NULL);

	gpio_write(NRF905_TRX_CE, 0);
	gpio_write(NRF905_TX_EN, 0);
	gpio_write(NRF905_PWR_UP, 1);
	// PWR_DWN -> ST_BY mode (3ms)
	timeout_add(&ctx->timer, 3, cb, data);
}

void
nrf905_data_ready_interrupt(void *data)
{
	// XXX we might need a NOP here, maybe a write config reg to address 10 will work
	nrf905_handle_state(data);
}

void
nrf905_set_rx_addr(struct nrf905_ctx_t *ctx, uint8_t *addr, uint8_t len, spi_cb *cb)
{
	for (int i = 0; i < len && i < 4; i++) {
		ctx->config.RX_ADDRESS[i] = addr[i];
	}
	nrf905_write_config(ctx, 5, cb, NULL);
}

void
nrf905_set_tx_addr(struct nrf905_ctx_t *ctx, uint8_t *addr, uint8_t len, spi_cb *cb)
{
	ctx->trans.cmd[0] = W_TX_ADDRESS;
	ctx->trans.cmd_len = 1;
	ctx->trans.tx_data = addr;
	ctx->trans.tx_len = len;
	ctx->trans.rx_data = NULL;
	ctx->trans.rx_len = 0;
	nrf905_send_command(ctx, cb, NULL);
}

void
nrf905_send(struct nrf905_ctx_t *ctx, void *data, uint8_t len, nrf905_data_callback cb)
{
	ctx->cb = cb;
	ctx->cb_data = data;
	ctx->cb_data_len = len;
	ctx->config.TX_PW = len;
	ctx->state = NRF905_TX_LEN;
	gpio_write(NRF905_TX_EN, 1);
	nrf905_write_config(ctx, 4, nrf905_handle_state, ctx);
}

void
nrf905_receive(struct nrf905_ctx_t *ctx, void *data, uint8_t len, nrf905_data_callback cb)
{
	ctx->cb = cb;
	ctx->cb_data = data;
	ctx->cb_data_len = len;
	ctx->config.RX_PW = len;
	ctx->state = NRF905_RX_LEN;
	nrf905_write_config(ctx, 3, nrf905_handle_state, ctx);
}

void
nrf905_set_channel(struct nrf905_ctx_t *ctx, uint8_t ch_no, enum nrf905_hfreq_pll_t hfreq_pll,
	enum nrf905_pa_pwr_t pa_pwr, spi_cb *cb)
{
	ctx->config.CH_NO = ch_no;
	ctx->config.HFREQ_PLL = hfreq_pll;
	ctx->config.PA_PWR = pa_pwr;
	memcpy(ctx->trans.cmd, &ctx->config, 2);
	ctx->trans.cmd[1] &= CHANNEL_CONFIG_MASK;
	ctx->trans.cmd_len = 2;
	ctx->trans.tx_data = NULL;
	ctx->trans.tx_len = 0;
	ctx->trans.rx_data = NULL;
	ctx->trans.rx_len = 0;
	nrf905_send_command(ctx, cb, NULL);
}

void
nrf905_save_config(struct nrf905_ctx_t *ctx, spi_cb *cb)
{
	nrf905_write_config(ctx, 0, cb, NULL);
}

void
nrf905_load_config(struct nrf905_ctx_t *ctx, spi_cb *cb)
{
	nrf905_read_config(ctx, 0, cb, NULL);
}

void
nrf905_init(struct nrf905_ctx_t *ctx, spi_cb *cb)
{
	pin_mode(NRF905_CSN, PIN_MODE_MUX_ALT2);
	pin_mode(NRF905_SCK, PIN_MODE_MUX_ALT2);
	pin_mode(NRF905_MOSI, PIN_MODE_MUX_ALT2);
	pin_mode(NRF905_MISO, PIN_MODE_MUX_ALT2);
	gpio_dir(NRF905_DR, GPIO_INPUT);
	gpio_dir(NRF905_TX_EN, GPIO_OUTPUT);
	gpio_dir(NRF905_TRX_CE, GPIO_OUTPUT);
	gpio_dir(NRF905_PWR_UP, GPIO_OUTPUT);
	nrf905_reset(ctx, cb, NULL);
}