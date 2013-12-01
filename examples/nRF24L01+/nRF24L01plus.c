#include <mchck.h>
#include "nRF24L01plus.h"

#define NRF_REG_MASK 0x1f // 0001 1111

/* nrf spi commands */
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

/* nrf registers */
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
	NRF_SPI_CS = SPI_PCS2
};

/* nrf app state */
enum nrf_state_t {
	// recv
	NRF_STATE_RECV_SET_RFSETUP,
	NRF_STATE_RECV_SET_CHANNEL,
	NRF_STATE_RECV_SET_PAYLOAD_SIZE,
	NRF_STATE_RECV_SET_RX_ADDRESS,
	NRF_STATE_RECV_SET_RX_HIGH,
	NRF_STATE_RECV_SET_CE_HIGH,
	NRF_STATE_RECV_WAITING,
	NRF_STATE_RECV_FETCH_DATA,
	NRF_STATE_RECV_USER_DATA,
	// send
	NRF_STATE_SEND_SET_RFSETUP,
	NRF_STATE_SEND_SET_CHANNEL,
	NRF_STATE_SEND_SET_RX_LOW,
	NRF_STATE_SEND_SET_TX_ADDR,
	NRF_STATE_SEND_SET_RX_ADDR_P0,
	NRF_STATE_SEND_TX_PAYLOAD,
	NRF_STATE_SEND_SET_CE_HIGH,
	NRF_STATE_SEND_WAITING,
	NRF_STATE_SEND_DATA_SENT,
	NRF_STATE_SEND_MAX_RT,
	NRF_STATE_SEND_TX_FLUSHED
};

/* BEGIN nrf registers */
struct nrf_datapipe_payload_size_t {
	uint8_t size : 6; // 1...32, 0 pipe not used
	uint8_t pad : 2; // 0
} __packed;
CTASSERT_SIZE_BYTE(struct nrf_datapipe_payload_size_t, 1);

struct nrf_status_t {
	uint8_t TX_FULL : 1;
	uint8_t RX_P_NO : 3; // reset value 111
	uint8_t MAX_RT : 1;
	uint8_t TX_DS : 1;
	uint8_t RX_DR : 1;
	uint8_t pad : 1; // 0
} __packed;
CTASSERT_SIZE_BYTE(struct nrf_status_t, 1);

struct nrf_rf_ch_t {
	uint8_t RF_CH : 7; // reset value 0000010
	uint8_t pad : 1; // 0
} __packed;
CTASSERT_SIZE_BYTE(struct nrf_rf_ch_t, 1);

struct nrf_rf_setup_t {
	uint8_t pad2 : 1; // 0
	enum nrf_tx_output_power_t power : 2;
	enum nrf_data_rate_t rate : 3;
	uint8_t pad1 : 1; // 0
	uint8_t CONT_WAVE : 1;
} __packed;
CTASSERT_SIZE_BYTE(struct nrf_rf_setup_t, 1);

struct nrf_addr_width_t {
	enum nrf_rxtx_addr_field_width {
		NRF_ADDR_FIELD_WIDTH_ILLEGAL = 0,
		NRF_ADDR_FIELD_WIDTH_3_BYTES = 1,
		NRF_ADDR_FIELD_WIDTH_4_BYTES = 2,
		NRF_ADDR_FIELD_WIDTH_5_BYTES = 3 // reset value
	} width : 2;
	uint8_t pad : 6; // 000000
} __packed;
CTASSERT_SIZE_BYTE(struct nrf_addr_width_t, 1);

struct nrf_retries_t {
	uint8_t ARC : 4; // number of retries, reset value 3
	uint8_t ARD : 4; // in 250us steps, reset value 0
} __packed;
CTASSERT_SIZE_BYTE(struct nrf_retries_t, 1);

/* END nrf registers */

struct nrf_transaction_t {
	struct spi_ctx_bare spi_ctx;
	struct sg tx_sg[2];
	struct sg rx_sg[2];
	uint8_t cmd;
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
	uint8_t channel;
	enum nrf_data_rate_t data_rate;
	enum nrf_tx_output_power_t power;
	enum nrf_crc_encoding_scheme crc_len;
};

static uint8_t btle_on = 0;

static struct nrf_context_t nrf_ctx;

static void nrf_handle_receive(void*);
static void nrf_handle_send(void*);

#define NRF_SET_CTX(_cmd, _tx_len, _tx_data, _rx_len, _rx_data, _state)	\
	nrf_ctx.trans.cmd = _cmd;											\
	nrf_ctx.trans.tx_len = _tx_len;										\
	nrf_ctx.trans.tx_data = _tx_data;									\
	nrf_ctx.trans.rx_len = _rx_len;										\
	nrf_ctx.trans.rx_data = _rx_data;									\
	nrf_ctx.state = _state;												\

static void
send_command(struct nrf_transaction_t *trans, spi_cb *cb, void *data)
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
handle_status(void *data)
{
	struct nrf_transaction_t *trans = data;

	// HAAAAACK, just to be sure
	if (btle_on)
		goto handle_status_cli;

	if (trans->status.RX_DR && nrf_ctx.state == NRF_STATE_RECV_WAITING) {
		nrf_ctx.state = NRF_STATE_RECV_FETCH_DATA;
		nrf_handle_receive(NULL);
	}

	if (trans->status.TX_DS && nrf_ctx.state == NRF_STATE_SEND_WAITING) {
		nrf_ctx.state = NRF_STATE_SEND_DATA_SENT;
		nrf_handle_send(NULL);
	}

	if (trans->status.MAX_RT && nrf_ctx.state == NRF_STATE_SEND_WAITING) {
		nrf_ctx.state = NRF_STATE_SEND_MAX_RT;
		nrf_handle_send(NULL);
	}

handle_status_cli:
	// clear NRF interrupt
	trans->cmd = NRF_CMD_W_REGISTER | (NRF_REG_MASK & NRF_REG_ADDR_STATUS);
	trans->tx_len = 1;
	trans->tx_data = &trans->status;
	trans->rx_len = 0;
	send_command(trans, NULL, NULL);
}

void
PORTC_Handler(void)
{
	gpio_write(NRF_CE, 0);
	// onboard_led(ONBOARD_LED_TOGGLE);
	pin_physport_from_pin(NRF_IRQ)->pcr[pin_physpin_from_pin(NRF_IRQ)].raw |= 0; // clear MCU interrupt
	static struct nrf_transaction_t trans = {
		.cmd = NRF_CMD_NOP,
		.tx_len = 0,
		.rx_len = 0
	};
	send_command(&trans, handle_status, &trans); // nop it to get the status register
}

void
nrf_init(void)
{
	nrf_ctx.channel = 16;
	nrf_ctx.data_rate = NRF_DATA_RATE_1MBPS;
	nrf_ctx.power = NRF_TX_POWER_0DBM;
	nrf_ctx.crc_len = NRF_CRC_ENC_1_BYTE;

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
}

void
nrf_receive(struct nrf_addr_t *addr, void *data, uint8_t len, nrf_data_callback cb)
{
	nrf_ctx.rx_addr = addr;
	nrf_ctx.rx_pipe = 1;
	nrf_ctx.payload_size = len;
	nrf_ctx.payload = data;
	nrf_ctx.user_cb = cb;
	nrf_ctx.state = NRF_STATE_RECV_SET_RX_HIGH;
	nrf_handle_receive(NULL);
}

void
nrf_send(struct nrf_addr_t *addr, void *data, uint8_t len, nrf_data_callback cb)
{
	nrf_ctx.tx_addr = addr;
	nrf_ctx.payload_size = len;
	nrf_ctx.payload = data;
	nrf_ctx.user_cb = cb;
	nrf_ctx.state = NRF_STATE_SEND_SET_RX_LOW;
	nrf_handle_send(NULL);
}

void
nrf_read_register(uint8_t reg, nrf_callback cb)
{
	static uint8_t data;
	nrf_ctx.trans.cmd = NRF_CMD_R_REGISTER | (NRF_REG_MASK & reg);
	nrf_ctx.trans.tx_len = 0;
	nrf_ctx.trans.tx_data = NULL;
	nrf_ctx.trans.rx_len = 1;
	nrf_ctx.trans.rx_data = &data;
	send_command(&nrf_ctx.trans, cb, &data);
}

void
nrf_write_register(uint8_t reg, void *data, size_t len, nrf_callback cb)
{
	nrf_ctx.trans.cmd = NRF_CMD_W_REGISTER | (NRF_REG_MASK & reg);
	nrf_ctx.trans.tx_len = len;
	nrf_ctx.trans.tx_data = data;
	nrf_ctx.trans.rx_len = 0;
	nrf_ctx.trans.rx_data = NULL;
	send_command(&nrf_ctx.trans, cb, &data);
}

static void
nrf_handle_receive(void *data)
{
	switch (nrf_ctx.state) {
	default:
	case NRF_STATE_RECV_SET_RX_HIGH: {
		static struct nrf_reg_config_t config = {
			.pad = 0,
			.PRIM_RX = NRF_PRIM_RX_PRX,
			.PWR_UP = 1,
			.MASK_MAX_RT = 0,
			.MASK_TX_DS = 0,
			.MASK_RX_DR = 0
		};
		config.CRCO = nrf_ctx.crc_len;
		NRF_SET_CTX(NRF_CMD_W_REGISTER | (NRF_REG_MASK & NRF_REG_ADDR_CONFIG),
			1, &config,
			0, NULL,
			NRF_STATE_RECV_SET_RFSETUP);
		break;
	}
	case NRF_STATE_RECV_SET_RFSETUP: {
		static struct nrf_rf_setup_t rfsetup = {
			.pad2 = 0,
			.pad1 = 0,
			.CONT_WAVE = 0
		};
		rfsetup.power = nrf_ctx.power;
		rfsetup.rate = nrf_ctx.data_rate;
		NRF_SET_CTX(NRF_CMD_W_REGISTER | (NRF_REG_MASK & NRF_REG_ADDR_RF_SETUP),
			1, &rfsetup,
			0, NULL,
			NRF_STATE_RECV_SET_CHANNEL);
		break;
	}
	case NRF_STATE_RECV_SET_CHANNEL: {
		static struct nrf_rf_ch_t ch = {
			.pad = 0
		};
		ch.RF_CH = nrf_ctx.channel;
		NRF_SET_CTX(NRF_CMD_W_REGISTER | (NRF_REG_MASK & NRF_REG_ADDR_RF_CH),
			1, &ch,
			0, NULL,
			NRF_STATE_RECV_SET_PAYLOAD_SIZE);
		break;
	}
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
			nrf_ctx.rx_addr->size, &nrf_ctx.rx_addr->value,
			0, NULL,
			NRF_STATE_RECV_SET_CE_HIGH);
		break;
	case NRF_STATE_RECV_SET_CE_HIGH:
		gpio_write(NRF_CE, 1);
		nrf_ctx.state = NRF_STATE_RECV_WAITING;
		// FALLTHROUGH
	case NRF_STATE_RECV_WAITING:
		return; // wait for interrupt
	case NRF_STATE_RECV_FETCH_DATA:
		// gpio_write(NRF_CE, 0);
		NRF_SET_CTX(NRF_CMD_R_RX_PAYLOAD,
			0, NULL,
			nrf_ctx.payload_size, nrf_ctx.payload,
			NRF_STATE_RECV_USER_DATA);
		break;
	case NRF_STATE_RECV_USER_DATA:
		nrf_ctx.user_cb(nrf_ctx.rx_addr, nrf_ctx.payload, nrf_ctx.payload_size);
		return; // the end
	}
	send_command(&nrf_ctx.trans, nrf_handle_receive, NULL);
}

static void
nrf_handle_send(void *data)
{
	switch (nrf_ctx.state) {
	default:
	case NRF_STATE_SEND_SET_RX_LOW: {
		static struct nrf_reg_config_t config = {
			.pad = 0,
			.PRIM_RX = NRF_PRIM_RX_PTX,
			.PWR_UP = 1,
			.MASK_MAX_RT = 0,
			.MASK_TX_DS = 0,
			.MASK_RX_DR = 0
		};
		config.CRCO = nrf_ctx.crc_len;
		NRF_SET_CTX(NRF_CMD_W_REGISTER | (NRF_REG_MASK & NRF_REG_ADDR_CONFIG),
			1, &config,
			0, NULL,
			NRF_STATE_SEND_SET_RFSETUP);
		break;
	}
	case NRF_STATE_SEND_SET_RFSETUP: {
		static struct nrf_rf_setup_t rfsetup = {
			.pad2 = 0,
			.pad1 = 0,
			.CONT_WAVE = 0
		};
		rfsetup.power = nrf_ctx.power;
		rfsetup.rate = nrf_ctx.data_rate;
		NRF_SET_CTX(NRF_CMD_W_REGISTER | (NRF_REG_MASK & NRF_REG_ADDR_RF_SETUP),
			1, &rfsetup,
			0, NULL,
			NRF_STATE_SEND_SET_CHANNEL);
		break;
	}
	case NRF_STATE_SEND_SET_CHANNEL: {
		static struct nrf_rf_ch_t ch = {
			.pad = 0
		};
		ch.RF_CH = nrf_ctx.channel;
		NRF_SET_CTX(NRF_CMD_W_REGISTER | (NRF_REG_MASK & NRF_REG_ADDR_RF_CH),
			1, &ch,
			0, NULL,
			NRF_STATE_SEND_SET_TX_ADDR);
		break;
	}
	case NRF_STATE_SEND_SET_TX_ADDR: {
		NRF_SET_CTX(NRF_CMD_W_REGISTER | (NRF_REG_MASK & NRF_REG_ADDR_TX_ADDR),
			nrf_ctx.tx_addr->size, &nrf_ctx.tx_addr->value,
			0, NULL,
			NRF_STATE_SEND_SET_RX_ADDR_P0);
		break;
	}
	case NRF_STATE_SEND_SET_RX_ADDR_P0: {
		NRF_SET_CTX(NRF_CMD_W_REGISTER | (NRF_REG_MASK & NRF_REG_ADDR_RX_ADDR_P0),
			nrf_ctx.tx_addr->size, &nrf_ctx.tx_addr->value,
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
		nrf_ctx.user_cb(nrf_ctx.tx_addr, nrf_ctx.payload, nrf_ctx.payload_size);
		return;
	case NRF_STATE_SEND_MAX_RT:
		NRF_SET_CTX(NRF_CMD_FLUSH_TX,
			0, NULL,
			0, NULL,
			NRF_STATE_SEND_TX_FLUSHED);
		break;
	case NRF_STATE_SEND_TX_FLUSHED:
		nrf_ctx.user_cb(NULL, nrf_ctx.payload, nrf_ctx.payload_size);
		return;
	}
	send_command(&nrf_ctx.trans, nrf_handle_send, NULL);
}


/////////////////////////////////////////////////////////////////////////////////////////
// http://dmitry.gr/index.php?r=05.Projects&proj=15&proj=11.%20Bluetooth%20LE%20fakery
// https://github.com/floe/BTLE/blob/master/BTLE.cpp
/////////////////////////////////////////////////////////////////////////////////////////


enum nrf_btle_state_t {
  NRF_BTLE_STATE_INIT_SIMPLE,
  NRF_BTLE_STATE_SET_TX_ADDR,
  NRF_BTLE_STATE_SET_RX_ADDR_P0,
  NRF_BTLE_STATE_READY,
  NRF_BTLE_STATE_PREPARE_ADVERTISE,
  NRF_BTLE_STATE_ADVERTISE_SEQUENCE,
  NRF_BTLE_STATE_ADVERTISE_DATA,
  NRF_BTLE_STATE_ADVERTISE_PTX_UP,
  NRF_BTLE_STATE_ADVERTISE_CE_HIGH,
  NRF_BTLE_STATE_ADVERTISE_WAIT
};

struct nrf_btle_ctx_t {
  struct nrf_transaction_t trans;
  char *name;
  size_t name_len;
  uint8_t current_ch;
  void *payload;
  size_t payload_len;
  enum nrf_btle_state_t state;
};

#define NRF_BTLE_CH_COUNT 3

const uint8_t btle_channel[NRF_BTLE_CH_COUNT] = { 37, 38, 39 };
const uint8_t btle_frequency[NRF_BTLE_CH_COUNT] = { 2, 26, 80 };

// inverted 0x8E89BED6
static struct nrf_addr_t btle_ad_addr = {
	// .value =  { 0x6B, 0x7D, 0xad, 0x91, 0x71 },
	.value =  0x6B7Dad9171,
	.size = 4
};

static struct nrf_btle_ctx_t nrf_btle_ctx;

static void btLeCrc(const uint8_t* data, uint8_t len, uint8_t* dst) {
	uint8_t v, t, d;

	while (len--) {
		d = *data++;
		for (v = 0; v < 8; v++, d >>= 1) {
			t = dst[0] >> 7;
			dst[0] <<= 1;
			if (dst[1] & 0x80)
				dst[0] |= 1;
			dst[1] <<= 1;
			if (dst[2] & 0x80)
				dst[1] |= 1;
			dst[2] <<= 1;
			if (t != (d & 1)) {
				dst[2] ^= 0x5B;
				dst[1] ^= 0x06;
			}
		}
	}
}

static void btLeWhiten(uint8_t* data, uint8_t len, uint8_t whitenCoeff) {
	uint8_t  m;

	while (len--) {
		for (m = 1; m; m <<= 1) {
			if (whitenCoeff & 0x80) {
				whitenCoeff ^= 0x11;
				(*data) ^= m;
			}
			whitenCoeff <<= 1;
		}
		data++;
	}
}

// http://graphics.stanford.edu/~seander/bithacks.html#ReverseByteWith32Bits
static uint8_t swapbits(uint8_t b) {
	return ((b * 0x0802LU & 0x22110LU) | (b * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16;
}

static inline uint8_t btLeWhitenStart(uint8_t chan) {
	//the value we actually use is what BT'd use left shifted one...makes our life easier
	return swapbits(chan) | 2;
}

static void btLePacketEncode(uint8_t* packet, uint8_t len, uint8_t chan) {
	//length is of packet, including crc. pre-populate crc in packet with initial crc value!
	uint8_t i, dataLen = len - 3;

	btLeCrc(packet, dataLen, packet + dataLen);
	for (i = 0; i < 3; i++, dataLen++)
		packet[dataLen] = swapbits(packet[dataLen]);
	btLeWhiten(packet, len, btLeWhitenStart(chan));
	for(i = 0; i < len; i++)
		packet[i] = swapbits(packet[i]);
}

struct nrf_simple_cmd_t {
	uint8_t cmd;
	uint8_t data;
};

#define BTLE_INIT_SIMPLE_COUNT 11
static uint8_t init_sequence_simple_idx = 0;
static struct nrf_simple_cmd_t init_sequence[BTLE_INIT_SIMPLE_COUNT] = {
    { 0x20, 0x12 },        //on, no crc, int on RX/TX done
    { 0x21, 0x00 },        //no auto-acknowledge
    { 0x22, 0x00 },        //no RX
    { 0x23, 0x02 },        //5-byte address
    { 0x24, 0x00 },        //no auto-retransmit
    { 0x26, 0x06 },        //1MBps at 0dBm
    { 0x27, 0x3E },        //clear various flags
    { 0x3C, 0x00 },        //no dynamic payloads
    { 0x3D, 0x00 },        //no features
    { 0x31, 32 },          //always RX 32 bytes
    { 0x22, 0x01 },        //RX on pipe 0
};

#define BTLE_AD_SIMPLE_COUNT 11
static uint8_t ad_sequence_simple_idx = 0;
static struct nrf_simple_cmd_t ad_sequence[BTLE_AD_SIMPLE_COUNT] = {
	{ 0x25, 0x00 },
	{ 0x27, 0x6e },
	{ 0xe2, 0xff }, // 0xff HACK
	{ 0xe1, 0xff }, // 0xff HACK
};

static struct timeout_ctx ad_timeout;

static void
nrf_btle_handler_simple(void* data) {
	static uint8_t buf[32];
	static uint8_t i, L = 0;

	switch (nrf_btle_ctx.state) {
	case NRF_BTLE_STATE_INIT_SIMPLE: {
		struct nrf_simple_cmd_t *sc = &init_sequence[init_sequence_simple_idx];
		nrf_btle_ctx.trans.cmd = sc->cmd;
		nrf_btle_ctx.trans.tx_len = 1;
		nrf_btle_ctx.trans.tx_data = &sc->data;
		nrf_btle_ctx.trans.rx_len = 0;
		nrf_btle_ctx.trans.rx_data = NULL;
		nrf_btle_ctx.state = ++init_sequence_simple_idx == BTLE_INIT_SIMPLE_COUNT ? NRF_BTLE_STATE_SET_TX_ADDR : NRF_BTLE_STATE_INIT_SIMPLE;
		break;
	}
	case NRF_BTLE_STATE_SET_TX_ADDR:
		nrf_btle_ctx.trans.cmd = NRF_REG_ADDR_TX_ADDR;
		nrf_btle_ctx.trans.tx_len = btle_ad_addr.size;
		nrf_btle_ctx.trans.tx_data = &btle_ad_addr.value;
		nrf_btle_ctx.state = NRF_BTLE_STATE_SET_RX_ADDR_P0;
		break;
	case NRF_BTLE_STATE_SET_RX_ADDR_P0:
		nrf_btle_ctx.trans.cmd = NRF_REG_ADDR_RX_ADDR_P0;
		nrf_btle_ctx.state = NRF_BTLE_STATE_READY;
		break;
	case NRF_BTLE_STATE_READY:
		btle_on = 1;
		return;
	case NRF_BTLE_STATE_PREPARE_ADVERTISE: {
		buf[L++] = 0x40;        //PDU type, given address is random
		buf[L++] = 17 + nrf_btle_ctx.payload_len + 2; // 17 bytes of payload + data + 2
		buf[L++] = 0xde;
		buf[L++] = 0xad;
		buf[L++] = 0xbe;
		buf[L++] = 0xef;
		buf[L++] = 0xab;
		buf[L++] = 0xba;
		buf[L++] = 2;
		buf[L++] = 0x01;
		buf[L++] = 0x05;

		buf[L++] = 1 + nrf_btle_ctx.name_len; // 1 + name len;
        buf[L++] = 0x08;
		for (i = 0; i < nrf_btle_ctx.name_len; i++)
			buf[L++] = nrf_btle_ctx.name[i];

		buf[L++] = 1 + nrf_btle_ctx.payload_len; // 1 + data len
        buf[L++] = 0xff;
        uint8_t *data = nrf_btle_ctx.payload;
		for (i = 0; i < nrf_btle_ctx.payload_len; i++)
			buf[L++] = data[i];

        buf[L++] = 0x55;        //CRC start value: 0x555555
        buf[L++] = 0x55;
        buf[L++] = 0x55;

		if(++nrf_btle_ctx.current_ch == 3)
			nrf_btle_ctx.current_ch = 0;

        btLePacketEncode(buf, L, btle_channel[nrf_btle_ctx.current_ch]);
        nrf_btle_ctx.state = NRF_BTLE_STATE_ADVERTISE_SEQUENCE;
        // default no recv
		nrf_btle_ctx.trans.rx_len = 0;
		nrf_btle_ctx.trans.rx_data = NULL;
		break;
	}
	case NRF_BTLE_STATE_ADVERTISE_SEQUENCE: {
		struct nrf_simple_cmd_t *sc = &ad_sequence[ad_sequence_simple_idx];
		nrf_btle_ctx.trans.cmd = sc->cmd;
		nrf_btle_ctx.trans.tx_len = sc->data == 0xff ? 0 : 1; // 0xff HACK
		nrf_btle_ctx.trans.tx_data = &sc->data;
		nrf_btle_ctx.state = ++ad_sequence_simple_idx == BTLE_AD_SIMPLE_COUNT ? NRF_BTLE_STATE_ADVERTISE_DATA : NRF_BTLE_STATE_ADVERTISE_SEQUENCE;
		break;
	}
	case NRF_BTLE_STATE_ADVERTISE_DATA:
		nrf_btle_ctx.trans.cmd = NRF_CMD_W_TX_PAYLOAD;
		nrf_btle_ctx.trans.tx_len = L + 1;
		nrf_btle_ctx.trans.tx_data = buf;
		nrf_btle_ctx.state = NRF_BTLE_STATE_ADVERTISE_PTX_UP;
		break;
	case NRF_BTLE_STATE_ADVERTISE_PTX_UP: {
		static struct nrf_reg_config_t config = {
			.pad = 0,
			.PRIM_RX = NRF_PRIM_RX_PTX,
			.PWR_UP = 1
		};
		nrf_btle_ctx.trans.cmd = NRF_CMD_W_REGISTER | (NRF_REG_MASK & NRF_REG_ADDR_CONFIG);
		nrf_btle_ctx.trans.tx_len = 1;
		nrf_btle_ctx.trans.tx_data = &config;
		nrf_btle_ctx.state = NRF_BTLE_STATE_ADVERTISE_CE_HIGH;
		break;
	}
	case NRF_BTLE_STATE_ADVERTISE_CE_HIGH:
		gpio_write(NRF_CE, 1);
		nrf_btle_ctx.state = NRF_BTLE_STATE_ADVERTISE_WAIT;
		timeout_add(&ad_timeout, 10, nrf_btle_handler_simple, NULL); // way too much
		return;
	case NRF_BTLE_STATE_ADVERTISE_WAIT:
		gpio_write(NRF_CE, 0);
		return;
	}
	send_command(&nrf_btle_ctx.trans, nrf_btle_handler_simple, NULL);
}

void nrf_btle_init(char* name, size_t name_len)
{
	nrf_btle_ctx.current_ch = 0;
	nrf_btle_ctx.name = name;
	nrf_btle_ctx.name_len = name_len;
	nrf_btle_ctx.state = NRF_BTLE_STATE_INIT_SIMPLE;
	nrf_btle_handler_simple(NULL);
}

void nrf_btle_advertise(void* data, size_t len)
{
	if (!btle_on) return;
	nrf_btle_ctx.payload = data;
	nrf_btle_ctx.payload_len = len;
	nrf_btle_ctx.state = NRF_BTLE_STATE_PREPARE_ADVERTISE;
	nrf_btle_handler_simple(NULL);
}
