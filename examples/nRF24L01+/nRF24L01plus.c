#include <mchck.h>
#include "nRF24L01plus.h"


#define NRF_REG_MASK 0x1f


enum NRF_CMD {
	NRF_CMD_R_REGISTER 			= 0x0a, // 5 lower bits masked
	NRF_CMD_W_REGISTER 			= 0x1a, // 5 lower bits masked
	NRF_CMD_R_RX_PAYLOAD 		= 0x61,
	NRF_CMD_W_TX_PAYLOAD 		= 0xa0,
	NRF_CMD_FLUSH_TX 			= 0xe1,
	NRF_CMD_FLUSH_RX 			= 0xe2,
	NRF_CMD_REUSE_TX_PL 		= 0xe3,
	NRF_CMD_R_RX_PL_WID 		= 0x60,
	NRF_CMD_W_ACK_PAYLOAD 		= 0xa8, // 3 lower bits masked
	NRF_CMD_W_TX_PAYLOAD_NO_ACK = 0xb0,
	NRF_CMD_NOP 				= 0xff
};

enum NRF_REG_ADDR {
	NRF_REG_ADDR_CONFIG 		= 0x00,
	NRF_REG_ADDR_EN_AA 			= 0x01,
	NRF_REG_ADDR_EN_RXADDR 		= 0x02,
	NRF_REG_ADDR_SETUP_AW 		= 0x03,
	NRF_REG_ADDR_SETUP_RETR 	= 0x04,
	NRF_REG_ADDR_RF_CH 			= 0x05,
	NRF_REG_ADDR_RF_SETUP 		= 0x06,
	NRF_REG_ADDR_STATUS 		= 0x07,
	NRF_REG_ADDR_OBSERVE_TX 	= 0x08,
	NRF_REG_ADDR_RPD 			= 0x09,
	NRF_REG_ADDR_RX_ADDR_P0 	= 0x0a,
	NRF_REG_ADDR_RX_ADDR_P1 	= 0x0b,
	NRF_REG_ADDR_RX_ADDR_P2 	= 0x0c,
	NRF_REG_ADDR_RX_ADDR_P3 	= 0x0d,
	NRF_REG_ADDR_RX_ADDR_P4 	= 0x0e,
	NRF_REG_ADDR_RX_ADDR_P5 	= 0x0f,
	NRF_REG_ADDR_TX_ADDR 		= 0x10,
	NRF_REG_ADDR_RX_PW_P0 		= 0x11,
	NRF_REG_ADDR_RX_PW_P1 		= 0x12,
	NRF_REG_ADDR_RX_PW_P2 		= 0x13,
	NRF_REG_ADDR_RX_PW_P3 		= 0x14,
	NRF_REG_ADDR_RX_PW_P4 		= 0x15,
	NRF_REG_ADDR_RX_PW_P5 		= 0x16,
	NRF_REG_ADDR_FIFO_STATUS 	= 0x17,
	NRF_REG_ADDR_DYNPD 			= 0x1c,
	NRF_REG_ADDR_FEATURE 		= 0x1d
};

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
	NRF_SPI_CS = SPI_PCS0
};

enum {
	NRF_CE_TX = 0,
	NRF_CE_RX = 1
};

struct nrf_datapipe_payload_size_t {
	uint8_t pad : 2; // 0
	uint8_t size : 5; // 1...32, 0 pipe not used
};

struct nrf_reg_config_t {
	uint8_t pad: 1; // 0
	uint8_t MASK_RX_DR : 1;
	uint8_t MASK_TX_DS : 1;
	uint8_t MASK_MAX_RT : 1;
	uint8_t EN_CRC : 1;  // reset value 1
	enum nrf_crc_encoding_scheme {
		NRF_CRC_ENC_1_BYTE = 0,
		NRF_CRC_ENC_2_BYTES = 1
	} CRCO : 1;
	uint8_t PWR_UP : 1;
	enum nrf_rxtx_control {
		NRF_RX_MODE = 0, // reset value
		NRF_TX_MODE = 1
	} PRIM_RX : 1;
};

struct nrf_status_t {
	uint8_t pad : 1; // 0
	uint8_t RX_DR : 1;
	uint8_t TX_DS : 1;
	uint8_t MAX_RT : 1;
	uint8_t RX_P_NO : 3; // reset value 111
	uint8_t TX_FULL : 1;
};

struct nrf_rf_ch_t {
	uint8_t pad : 1; // 0
	uint8_t RF_CH : 7; // reset value 0000010
};

struct nrf_rf_setup_t {
	uint8_t CONT_WAVE : 1;
	uint8_t pad1 : 1; // 0
	enum nrf_data_rate_t rate : 3;
	enum nrf_tx_output_power_t RF_PWR : 2;
	uint8_t pad2 : 1; // 0
};

struct nrf_transaction_t {
	struct spi_ctx_bare spi_ctx;
	struct sg tx_sg[2];
	struct sg rx_sg[2];
	enum NRF_CMD cmd;
	uint8_t tx_len;
	void *tx_data;
	uint8_t rx_len;
	struct nrf_status_t status;
	void *rx_data;
};

struct nrf_context_t {
	struct nrf_transaction_t trans;
	nrf_data_callback user_cb;
	void *user_data;
	size_t user_data_len;
	enum nrf_state_t {
		NRF_TX,
		NRF_RX
	} state;
};

static nrf_context_t nrf_ctx;

static void
send_command(struct nrf_transaction_t *trans, spi_cb *cb)
{
	spi_queue_xfer_sg(&trans->spi_ctx, NRF_SPI_CS,
			sg_init(trans->tx_sg, &trans->cmd, 1, trans->tx_data, trans->tx_len),
			sg_init(trans->rx_sg, &trans->status, 1, trans->rx_data, trans->rx_len),
			cb, trans);
}

static void
nrf_set_power_rxtx(uint8_t up, uint8_t rx)
{
	static struct nrf_reg_config_t config = {
		.pad = 0
	};
	config.PRIM_RX = rx;
	config.PWR_UP = up;
	nrf_ctx.trans.cmd = NRF_CMD_W_REGISTER | (NRF_REG_MASK & NRF_REG_ADDR_CONFIG);
	nrf_ctx.trans.tx_len = 1;
	nrf_ctx.trans.tx_data = &config;
	nrf_ctx.trans.rx_len = 0;
	send_command(&nrf_ctx.trans, NULL);
}

static void
nrf_set_datapipe_payload_size(uint8_t pipe, uint8_t size)
{
	static struct nrf_datapipe_payload_size_t dps = {
		.pad = 0
	};
	nrf_ctx.trans.cmd = NRF_CMD_W_REGISTER | (NRF_REG_MASK & (NRF_REG_ADDR_RX_PW_P0 + pipe));
	nrf_ctx.trans.tx_len = 1;
	nrf_ctx.trans.tx_data = &dps;
	nrf_ctx.trans.rx_len = 0;
	dps.size = size;
	send_command(&nrf_ctx.trans, NULL);
}

static void
nrf_set_rx_address(uint8_t pipe, struct nrf_addr_t *addr)
{
	nrf_ctx.trans.cmd = NRF_CMD_W_REGISTER | (NRF_REG_MASK & (NRF_REG_ADDR_RX_ADDR_P0 + pipe));
	nrf_ctx.trans.tx_len = addr->size;
	nrf_ctx.trans.tx_data = addr->value;
	nrf_ctx.trans.rx_len = 0;
	send_command(&nrf_ctx.trans, NULL);
}

static void
handle_read_payload(void *data)
{
	nrf_ctx.trans.user_cb(NULL, nrf_ctx.trans.user_data, nrf_ctx.trans.user_data_len);
}

static void
handle_status(void *data)
{
	struct nrf_transaction_t *trans = data;

	if (trans->status.RX_DR) {
		gpio_write(NRF_CE, NRF_CE_TX);
		nrf_ctx.trans.cmd = NRF_CMD_R_RX_PAYLOAD;
		nrf_ctx.trans.tx_len = 0;
		nrf_ctx.trans.rx_len = nrf_ctx.user_data_len;
		nrf_ctx.trans.rx_data = nrf_ctx.trans.user_data;
		send_command(&nrf_ctx.trans, handle_read_payload);
	}

	if (trans->status.TX_DS) {
		status.TX_DS = 1;
	}

	if (trans->status.MAX_RT) {
		status.MAX_RT = 1;
	}

	// clear interrupt
	trans.cmd = NRF_CMD_W_REGISTER | (NRF_REG_MASK & NRF_REG_ADDR_STATUS);
	trans.tx_len = 1;
	trans.tx_data = &trans->status;
	trans.rx_len = 0;
	send_command(&trans_ci, NULL);
}

void
PORTC_Handler(void)
{
	static struct nrf_transaction_t trans = {
		.cmd = NRF_CMD_NOP,
		.tx_len = 0,
		.rx_len = 0
	};
	send_command(&trans, handle_status);
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
	gpio_write(NRF_CE, NRF_CE_TX);

	pin_mode(NRF_IRQ, PIN_MODE_PULLDOWN);
	pin_physport_from_pin(NRF_IRQ)->pcr[pin_physpin_from_pin(NRF_IRQ)].irqc = PCR_IRQC_INT_RISING;
	int_enable(IRQ_PORTC);
}

void
nrf_receive(struct nrf_addr_t *addr, void *data, uint8_t len, nrf_data_callback cb)
{
	nrf_ctx.user_cb = cb;
	nrf_ctx.user_data = data;
	nrf_ctx.user_data_len = len;
	nrf_ctx.state = NRF_RX;

	nrf_set_datapipe_payload_size(0, len);
	nrf_set_rx_address(0, addr);
	send_command(&nrf_ctx.trans, NULL);
	nrf_set_power_rxtx(1, 1);
	gpio_write(NRF_CE, NRF_CE_RX);
}

void
nrf_send(struct nrf_addr_t *addr, void *data, uint8_t len, nrf_data_callback cb)
{
}

void
nrf_set_channel(uint8_t channel)
{
	static struct nrf_rf_ch_t rf_ch = {
		.pad = 0
	};
	nrf_ctx.trans.cmd = NRF_CMD_W_REGISTER | (NRF_REG_MASK & NRF_REG_ADDR_RF_CH);
	nrf_ctx.trans.tx_len = 1;
	nrf_ctx.trans.tx_data = &rf_ch;
	nrf_ctx.trans.rx_len = 0;
	rf_ch.RF_CH = channel;
	send_command(&nrf_ctx.trans, NULL);
}

void
nrf_set_rate_and_power(enum nrf_data_rate_t data_rate, enum nrf_tx_output_power_t output_power)
{
	static struct nrf_rf_setup_t rf_setup = {
		.CONT_WAVE = 0,
		.pad1 = 0,
		.pad2 = 0
	};
	nrf_ctx.trans.cmd = NRF_CMD_W_REGISTER | (NRF_REG_MASK & NRF_REG_ADDR_RF_SETUP)
	nrf_ctx.trans.tx_len = 1;
	nrf_ctx.trans.tx_data = &rf_setup;
	nrf_ctx.trans.rx_len = 0;
	rf_setup.RF_PWR = output_power;
	rf_setup.rate = data_rate;
	send_command(&nrf_ctx.trans, NULL);
}
