
enum nrf_tx_output_power_t {
	NRF_TX_POWER_18DBM = 0,
	NRF_TX_POWER_12DBM = 1,
	NRF_TX_POWER_6DBM = 2,
	NRF_TX_POWER_0DBM = 3 // reset value
};

// PLL_LOCK is always 0
enum nrf_data_rate_t {
	NRF_DATA_RATE_1MBPS = 0x0,
	NRF_DATA_RATE_2MBPS = 0x1,
	NRF_DATA_RATE_250KBPS = 0x4
};

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

/* nrf register mappings */

struct nrf_reg_config_t {
	enum nrf_rxtx_control {
		NRF_PRIM_RX_PTX = 0, // reset value
		NRF_PRIM_RX_PRX = 1
	} PRIM_RX : 1;
	uint8_t PWR_UP : 1;
	enum nrf_crc_encoding_scheme {
		NRF_CRC_ENC_1_BYTE = 0, // reset value
		NRF_CRC_ENC_2_BYTES = 1
	} CRCO : 1;
	uint8_t EN_CRC : 1;  // reset value 1
	uint8_t MASK_MAX_RT : 1;
	uint8_t MASK_TX_DS : 1;
	uint8_t MASK_RX_DR : 1;
	uint8_t pad: 1; // 0
} __packed;
CTASSERT_SIZE_BYTE(struct nrf_reg_config_t, 1);

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
		// NRF_ADDR_FIELD_WIDTH_ILLEGAL = 0,
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


void nrf_init(void);

void nrf_read_register(uint8_t reg, void *data, size_t len);
uint8_t nrf_read_register_byte(uint8_t reg);
void nrf_write_register(uint8_t reg, void *data, size_t len);
void nrf_write_register_byte(uint8_t reg, uint8_t value);

void nrf_set_channel(uint8_t channel);
uint8_t nrf_get_channel();
void nrf_set_power_rate(enum nrf_tx_output_power_t power, enum nrf_data_rate_t data_rate);
void nrf_set_payload_size(uint8_t pipe, uint8_t payload_size);
uint8_t nrf_get_payload_size(uint8_t pipe);
void nrf_set_rx_address(uint8_t pipe, uint64_t address, enum nrf_rxtx_addr_field_width address_size);
void nrf_set_tx_address(uint64_t address, enum nrf_rxtx_addr_field_width address_size);
void nrf_enable_auto_ack(uint8_t pipe);
uint8_t nrf_auto_ack_enabled(uint8_t pipe);
void nrf_enable_rx_address(uint8_t pipe);
uint8_t nrf_rx_address_enable(uint8_t pipe);
void nrf_set_address_width(enum nrf_rxtx_addr_field_width width);
uint8_t nrf_get_address_width();
void nrf_set_auto_retransmit(uint8_t delay, uint8_t count);
void nrf_enable_dynamic_payload(uint8_t pipe);
uint8_t nrf_dynamic_payload_enabled(uint8_t pipe);
