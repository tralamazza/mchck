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

struct nrf_addr_t {
	uint8_t value[5];
	uint8_t size;
} __packed;

typedef void (nrf_data_callback)(struct nrf_addr_t *, void *, uint8_t);
typedef void (nrf_callback)(void *);

void nrf_init(void);
void nrf_receive(struct nrf_addr_t *, void *, uint8_t, nrf_data_callback);
void nrf_send(struct nrf_addr_t *, void *, uint8_t, nrf_data_callback);
void nrf_read_register(uint8_t, nrf_callback);
void nrf_write_register(uint8_t, void*, size_t, nrf_callback);

void nrf_btle_init(char* name, size_t name_len);
void nrf_btle_advertise(void* data, size_t len);
