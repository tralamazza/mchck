#include <mchck.h>
#include "nRF24L01plus.h"

#define NRF_REG_MASK 0x1f // 0001 1111

enum NRF_CMD {
	NRF_CMD_R_REGISTER 			= 0x00, // 5 lower bits masked
	NRF_CMD_W_REGISTER 			= 0x20, // 5 lower bits masked
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

/* SPI */
enum {
	NRF_SPI_CS = SPI_PCS0
};

/* nrf app state */
enum nrf_state_t {
	// recv
	NRF_STATE_RECV_SET_PAYLOAD_SIZE,
	NRF_STATE_RECV_SET_RX_ADDRESS,
	NRF_STATE_RECV_SET_RX_HIGH,
	NRF_STATE_RECV_SET_CE_HIGH,
	NRF_STATE_RECV_WAITING,
	NRF_STATE_RECV_FETCH_DATA,
	NRF_STATE_RECV_USER_DATA,
	// send
	NRF_STATE_SEND_SET_RX_LOW,
	NRF_STATE_SEND_SET_TX_ADDR,
	NRF_STATE_SEND_SET_RX_ADDR_P0,
	NRF_STATE_SEND_TX_PAYLOAD,
	NRF_STATE_SEND_SET_CE_HIGH,
	NRF_STATE_SEND_WAITING,
	NRF_STATE_SEND_DATA_SENT
};

/* BEGIN nrf registers */
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
		NRF_CRC_ENC_1_BYTE = 0, // reset value
		NRF_CRC_ENC_2_BYTES = 1
	} CRCO : 1;
	uint8_t PWR_UP : 1;
	enum nrf_rxtx_control {
		NRF_PRIM_RX_PTX = 0, // reset value
		NRF_PRIM_RX_PRX = 1
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
/* END nrf registers */

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
	struct nrf_addr_t *tx_addr;
	struct nrf_addr_t *rx_addr;
	uint8_t rx_pipe;
	size_t payload_size;
	void *payload;
	nrf_data_callback *user_cb;
	enum nrf_state_t state;
};

static struct nrf_context_t nrf_ctx;

static void nrf_handle_receive();
static void nrf_handle_send();

#define NRF_SET_CTX(_cmd, _tx_len, _tx_data, _rx_len, _rx_data, _state)	\
	nrf_ctx.trans.cmd = _cmd;											\
	nrf_ctx.trans.tx_len = _tx_len;										\
	nrf_ctx.trans.tx_data = _tx_data;									\
	nrf_ctx.trans.rx_len = _rx_len;										\
	nrf_ctx.trans.rx_data = _rx_data;									\
	nrf_ctx.state = _state;												\

static void
send_command(struct nrf_transaction_t *trans, spi_cb *cb)
{
	spi_queue_xfer_sg(&trans->spi_ctx, NRF_SPI_CS,
		sg_init(trans->tx_sg, &trans->cmd, 1, trans->tx_data, trans->tx_len),
		sg_init(trans->rx_sg, &trans->status, 1, trans->rx_data, trans->rx_len),
		cb, trans);
}

static void
handle_status(void *data)
{
	struct nrf_transaction_t *trans = data;

	if (trans->status.RX_DR) {
		nrf_ctx.state = NRF_STATE_RECV_FETCH_DATA;
		nrf_handle_receive();
	}

	if (trans->status.TX_DS) {
		nrf_ctx.state = -1;
		nrf_handle_send();
	}

	if (trans->status.MAX_RT) {
	}

	// clear NRF interrupt
	trans->cmd = NRF_CMD_W_REGISTER | (NRF_REG_MASK & NRF_REG_ADDR_STATUS);
	trans->tx_len = 1;
	trans->tx_data = &trans->status;
	trans->rx_len = 0;
	send_command(trans, NULL);
}

void
PORTC_Handler(void)
{
	static struct nrf_transaction_t trans = {
		.cmd = NRF_CMD_NOP,
		.tx_len = 0,
		.rx_len = 0
	};
	send_command(&trans, handle_status); // nop it to get the status register
	pin_physport_from_pin(NRF_IRQ)->pcr[pin_physpin_from_pin(NRF_IRQ)].raw |= 0; // clear MCU interrupt
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

	pin_mode(NRF_IRQ, PIN_MODE_PULLDOWN);
	pin_physport_from_pin(NRF_IRQ)->pcr[pin_physpin_from_pin(NRF_IRQ)].irqc = PCR_IRQC_INT_RISING;
	int_enable(IRQ_PORTC);
}

void
nrf_receive(struct nrf_addr_t *addr, void *data, uint8_t len, nrf_data_callback cb)
{
	nrf_ctx.rx_addr = addr;
	nrf_ctx.rx_pipe = 0;
	nrf_ctx.payload_size = len;
	nrf_ctx.payload = data;
	nrf_ctx.user_cb = cb;
	nrf_ctx.state = NRF_STATE_RECV_SET_PAYLOAD_SIZE;
	nrf_handle_receive();
}

static void
nrf_handle_receive()
{
	switch (nrf_ctx.state) {
	default:
	case NRF_STATE_RECV_SET_PAYLOAD_SIZE: {
		static struct nrf_datapipe_payload_size_t dps = {
			.pad = 0
		};
		dps.size = nrf_ctx.payload_size;
		NRF_SET_CTX(NRF_CMD_W_REGISTER | (NRF_REG_MASK & (NRF_REG_ADDR_RX_PW_P0 + nrf_ctx.rx_pipe)),
			1, &dps,
			0, NULL,
			NRF_STATE_RECV_SET_RX_ADDRESS);
		break;
	}
	case NRF_STATE_RECV_SET_RX_ADDRESS:
		NRF_SET_CTX(NRF_CMD_W_REGISTER | (NRF_REG_MASK & (NRF_REG_ADDR_RX_ADDR_P0 + nrf_ctx.rx_pipe)),
			nrf_ctx.rx_addr->size, nrf_ctx.rx_addr->value,
			0, NULL,
			NRF_STATE_RECV_SET_RX_HIGH);
		break;
	case NRF_STATE_RECV_SET_RX_HIGH: {
		static struct nrf_reg_config_t config = {
			.pad = 0,
			.PRIM_RX = NRF_PRIM_RX_PRX,
			.PWR_UP = 1
		};
		NRF_SET_CTX(NRF_CMD_W_REGISTER | (NRF_REG_MASK & NRF_REG_ADDR_CONFIG),
			1, &config,
			0, NULL,
			NRF_STATE_RECV_SET_CE_HIGH);
		break;
	}
	case NRF_STATE_RECV_SET_CE_HIGH:
		gpio_write(NRF_CE, 1);
		nrf_ctx.state = NRF_STATE_RECV_WAITING;
		// FALLTHROUGH
	case NRF_STATE_RECV_WAITING:
		return; // wait for interrupt
	case NRF_STATE_RECV_FETCH_DATA:
		gpio_write(NRF_CE, 0);
		NRF_SET_CTX(NRF_CMD_R_RX_PAYLOAD,
			0, NULL,
			nrf_ctx.payload_size, nrf_ctx.payload,
			NRF_STATE_RECV_USER_DATA);
		break;
	case NRF_STATE_RECV_USER_DATA:
		nrf_ctx.user_cb(nrf_ctx.rx_addr, nrf_ctx.payload, nrf_ctx.payload_size);
		return; // the end
	}
	send_command(&nrf_ctx.trans, nrf_handle_receive);
}

void
nrf_send(struct nrf_addr_t *addr, void *data, uint8_t len, nrf_data_callback cb)
{
	nrf_ctx.tx_addr = addr;
	nrf_ctx.payload_size = len;
	nrf_ctx.payload = data;
	nrf_ctx.user_cb = cb;
	nrf_ctx.state = NRF_STATE_SEND_SET_RX_LOW;
	nrf_handle_send();
}

static void
nrf_handle_send()
{
	switch (nrf_ctx.state) {
	default:
	case NRF_STATE_SEND_SET_RX_LOW: {
		static struct nrf_reg_config_t config = {
			.pad = 0,
			.PRIM_RX = NRF_PRIM_RX_PTX,
			.PWR_UP = 1
		};
		NRF_SET_CTX(NRF_CMD_W_REGISTER | (NRF_REG_MASK & NRF_REG_ADDR_CONFIG),
			1, &config,
			0, NULL,
			NRF_STATE_SEND_SET_TX_ADDR);
		break;
	}
	case NRF_STATE_SEND_SET_TX_ADDR: {
		NRF_SET_CTX(NRF_REG_ADDR_TX_ADDR,
			nrf_ctx.tx_addr->size, nrf_ctx.tx_addr->value,
			0, NULL,
			NRF_STATE_SEND_SET_RX_ADDR_P0);
		break;
	}
	case NRF_STATE_SEND_SET_RX_ADDR_P0: {
		NRF_SET_CTX(NRF_REG_ADDR_RX_ADDR_P0,
			nrf_ctx.tx_addr->size, nrf_ctx.tx_addr->value,
			0, NULL,
			NRF_STATE_SEND_TX_PAYLOAD);
		break;
	}
	case NRF_STATE_SEND_TX_PAYLOAD:
		NRF_SET_CTX(NRF_CMD_W_TX_PAYLOAD,
			nrf_ctx.payload_size, nrf_ctx.payload,
			0, NULL,
			NRF_STATE_SEND_SET_CE_HIGH);
		break;
	case NRF_STATE_SEND_SET_CE_HIGH:
		gpio_write(NRF_CE, 1);
		nrf_ctx.state = NRF_STATE_SEND_WAITING;
		// FALLTHROUGH
	case NRF_STATE_SEND_WAITING:
		return;
	case NRF_STATE_SEND_DATA_SENT:
		gpio_write(NRF_CE, 0);
		nrf_ctx.user_cb(nrf_ctx.tx_addr, nrf_ctx.payload, nrf_ctx.payload_size);
		return;
	}
	send_command(&nrf_ctx.trans, nrf_handle_send);
}

/*
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
	nrf_ctx.trans.cmd = NRF_CMD_W_REGISTER | (NRF_REG_MASK & NRF_REG_ADDR_RF_SETUP);
	nrf_ctx.trans.tx_len = 1;
	nrf_ctx.trans.tx_data = &rf_setup;
	nrf_ctx.trans.rx_len = 0;
	rf_setup.RF_PWR = output_power;
	rf_setup.rate = data_rate;
	send_command(&nrf_ctx.trans, NULL);
}
*/