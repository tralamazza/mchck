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
};

typedef void (nrf_data_callback)(struct nrf_addr_t *, void *, uint8_t);

void nrf_init(void);
void nrf_receive(struct nrf_addr_t *, void *, uint8_t, nrf_data_callback);
void nrf_send(struct nrf_addr_t *, void *, uint8_t, nrf_data_callback);
void nrf_set_channel(uint8_t);
void nrf_set_rate_and_power(enum nrf_data_rate_t, enum nrf_tx_output_power_t);

/*
struct nrf_data_pipe_payload_size_t {
	uint8_t : 2;// 0
	uint8_t size : 5; // 0 pipe not used
};

struct nrf_register_map_t {
	union {
		struct {
			uint8_t : 1; // 0
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
		uint8_t config_reg;
	};
	union {
		struct {
			uint8_t : 2; // 00
			uint8_t ENAA_P5 : 1; // reset value 1
			uint8_t ENAA_P4 : 1; // reset value 1
			uint8_t ENAA_P3 : 1; // reset value 1
			uint8_t ENAA_P2 : 1; // reset value 1
			uint8_t ENAA_P1 : 1; // reset value 1
			uint8_t ENAA_P0 : 1; // reset value 1
		};
		uint8_t enable_auto_ack;
	};
	union {
		struct {
			uint8_t : 2; // 00
			uint8_t ERX_P5 : 1;
			uint8_t ERX_P4 : 1;
			uint8_t ERX_P3 : 1;
			uint8_t ERX_P2 : 1;
			uint8_t ERX_P1 : 1; // reset value 1
			uint8_t ERX_P0 : 1; // reset value 1
		};
		const uint8_t enabled_ex_addresses;
	};
	union {
		struct {
			uint8_t : 5; // 00000
			enum nrf_rxtx_addr_field_width {
				NRF_ADDR_FIELD_WIDTH_ILLEGAL = 0,
				NRF_ADDR_FIELD_WIDTH_3_BYTES = 1,
				NRF_ADDR_FIELD_WIDTH_4_BYTES = 2,
				NRF_ADDR_FIELD_WIDTH_5_BYTES = 3 // reset value
			} AW : 2;
		};
		uint8_t setup_address_widths;
	};
	union {
		struct {
			uint8_t ARD : 4; // in 250us steps
			uint8_t ARC : 4; // reset value 0011
		};
		uint8_t automatic_retransmition_setup;
	};
	union {
		struct {
			uint8_t : 1; // 0
			uint8_t RF_CH : 7; // reset value 0000010
		};
		uint8_t rf_channel;
	};
	union {
		struct {
			uint8_t CONT_WAVE : 1;
			uint8_t : 1; // 0
			uint8_t RF_DR_LOW : 1;
			uint8_t PLL_LOCK : 1;
			uint8_t RF_DR_HIGH : 1; // reset value 1
			enum nrf_tx_output_power {
				NRF_TX_POWER_18DBM = 0,
				NRF_TX_POWER_12DBM = 1,
				NRF_TX_POWER_6DBM = 2,
				NRF_TX_POWER_0DBM = 3 // reset value
			} RF_PWR : 2;
			uint8_t : 1; // 0
		};
		uint8_t rf_setup;
	};
	union {
		struct {
			uint8_t : 1; // 0
			uint8_t RX_DR : 1;
			uint8_t TX_DS : 1;
			uint8_t MAX_RT : 1;
			const uint8_t RX_P_NO : 3; // reset value 111
			const uint8_t TX_FULL : 1;
		};
		uint8_t status_register;
	};
	union {
		struct {
			const uint8_t PLOS_CNT : 4;
			const uint8_t ARC_CNT : 4;
		};
		uint8_t transmit_observe_register;
	};
	union {
		struct {
			const uint8_t : 7; // 0
			const uint8_t RPD : 1;
		};
		uint8_t received_power_detector;
	};

	uint8_t RX_ADDR_P0[5];
	uint8_t RX_ADDR_P1[5];
	uint8_t RX_ADDR_P2;
	uint8_t RX_ADDR_P3;
	uint8_t RX_ADDR_P4;
	uint8_t RX_ADDR_P5;

	uint8_t TX_ADDR[5];

	struct nrf_data_pipe_payload_size_t RX_PW_P[5];

	union {
		struct {
			uint8_t : 1; // 0
			const uint8_t TX_REUSE : 1;
			const uint8_t TX_FULL_FIFO : 1;
			const uint8_t TX_EMPTY : 1;
			uint8_t : 2; // 00
			const uint8_t RX_FULL : 1;
			const uint8_t RX_EMPTY : 1;
		};
		uint8_t fifo_status;
	};

	// ACK_PLD, TX_PLD, RX_PLD

	union {
		struct {
			uint8_t : 2; // 00
			uint8_t DPL_P5 : 1;
			uint8_t DPL_P4 : 1;
			uint8_t DPL_P3 : 1;
			uint8_t DPL_P2 : 1;
			uint8_t DPL_P1 : 1;
			uint8_t DPL_P0 : 1;
		};
		uint8_t enable_dynamic_payload_len;
	};
	union {
		struct {
			uint8_t : 5; // 00000
			uint8_t EN_DPL : 1;
			uint8_t EN_ACK_PAY : 1;
			uint8_t EN_DYN_ACK : 1;
		};
		uint8_t feature_register;
	};
};
*/

