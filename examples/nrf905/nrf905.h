#ifndef __NRF905_H
#define __NRF905_H

#include <mchck.h>
#include <sys/types.h>

enum nrf905_addr_len_t {
	ADDR_LEN_1BYTE	= 0x1,
	ADDR_LEN_2BYTES	= 0x2,
	ADDR_LEN_4BYTES	= 0x4
}; 

struct nrf905_rf_config_register {
	// byte 0 and 1
	uint16_t CH_NO		: 9;
	enum nrf905_hfreq_pll_t {
		HFREQ_PLL_433MHZ	= 0x0,
		HFREQ_PLL_868_915MHZ	= 0x1
	} HFREQ_PLL		: 1;
	enum nrf905_pa_pwr_t {
		PA_PWR_N10DBM	= 0x0,
		PA_PWR_N2DBM	= 0x1,
		PA_PWR_6DBM	= 0x2,
		PA_PWR_10DBM	= 0x3
	} PA_PWR		: 2;
	uint8_t RX_RED_PWR	: 1;
	uint8_t AUTO_RETRAN	: 1;
	uint8_t _pad1		: 2;
	// byte 2
	enum nrf905_addr_len_t RX_AFW	: 3;
	uint8_t _pad2		: 1;
	enum nrf905_addr_len_t TX_AFW	: 3;
	uint8_t _pad3		: 1;
	// byte 3
	uint8_t RX_PW		: 6;
	uint8_t _pad4		: 2;
	// byte 4
	uint8_t TX_PW		: 6;
	uint8_t _pad5		: 2;
	// byte 5
	uint8_t RX_ADDRESS[4];
	// byte 9
	enum {
		UP_CLK_FREQ_4MHZ	= 0x0,
		UP_CLK_FREQ_2MHZ	= 0x1,
		UP_CLK_FREQ_1MHZ	= 0x2,
		UP_CLK_FREQ_500KHZ	= 0x3
	} UP_CLK_FREQ		: 2;
	uint8_t UP_CLK_EN	: 1;
	enum {
		XOF_4MHZ	= 0x0,
		XOF_8MHZ	= 0x1,
		XOF_12MHZ	= 0x2,
		XOF_16MHZ	= 0x3,
		XOF_20MHZ	= 0x4
	} XOF			: 3;
	uint8_t CRC_EN		: 1;
	enum {
		CRC_MODE_8BITS	= 0x0,
		CRC_MODE_16BITS	= 0x1
	} CRC_MODE		: 1;
};
CTASSERT_SIZE_BYTE(struct nrf905_rf_config_register, 10);

struct nrf905_status_register {
	uint8_t _pad1	: 5;
	uint8_t DR	: 1;
	uint8_t _pad2	: 1;
	uint8_t AM	: 1;
};
CTASSERT_SIZE_BYTE(struct nrf905_status_register, 1);

typedef void (nrf905_data_callback)(void *, uint8_t);

struct nrf905_ctx_t {
	struct nrf905_transaction_t {
		struct spi_ctx_bare spi_ctx;
		struct sg tx_sg[2];
		struct sg rx_sg[2];
		uint8_t cmd[2];
		uint8_t cmd_len;
		uint8_t tx_len;
		void *tx_data;
		uint8_t rx_len;
		void *rx_data;
	} trans;
	nrf905_data_callback *cb;
	void *cb_data;
	uint8_t cb_data_len;
	struct nrf905_status_register status;
	struct nrf905_rf_config_register config;
	enum {
		NRF905_IDLE,
		NRF905_TX_LEN,
		NRF905_TX_PAYLOAD,
		NRF905_TX_DONE,
		NRF905_RX_LEN,
		NRF905_RX_PAYLOAD,
		NRF905_RX_DONE
	} state;
	struct timeout_ctx timer;
};

enum {
	NRF905_TX_EN	= PIN_PTB3,
	NRF905_PWR_UP	= PIN_PTB2,
	NRF905_TRX_CE	= PIN_PTB1,
	NRF905_DR	= PIN_PTB0,
	NRF905_CSN	= PIN_PTC2,
	NRF905_SCK	= PIN_PTC5,
	NRF905_MOSI	= PIN_PTC6,
	NRF905_MISO	= PIN_PTC7
};

void nrf905_data_ready_interrupt(void *cbdata);

/* setup nrf905 DR interrupt */
#define NRF905_INT_DECL(_ctx)	\
	PIN_DEFINE_CALLBACK(NRF905_DR, PIN_CHANGE_RISING, nrf905_data_ready_interrupt, _ctx);	\

/* initialization, call it once
   requires: timeout_init(), spit_init() and pin_change_init() */
void nrf905_init(struct nrf905_ctx_t *ctx, spi_cb *cb);

/* set RX address (1 or 4 bytes long) */
void nrf905_set_rx_addr(struct nrf905_ctx_t *ctx, uint8_t *addr, uint8_t len, spi_cb *cb);

/* set TX address (1 or 4 bytes long) */
void nrf905_set_tx_addr(struct nrf905_ctx_t *ctx, uint8_t *addr, uint8_t len, spi_cb *cb);

/* send "len" bytes of "data" */
void nrf905_send(struct nrf905_ctx_t *ctx, void *data, uint8_t len, nrf905_data_callback cb);

/* receive "len" bytes of "data" */
void nrf905_receive(struct nrf905_ctx_t *ctx, void *data, uint8_t len, nrf905_data_callback cb);

/* reset the IC and load defaults */
void nrf905_reset(struct nrf905_ctx_t *ctx, spi_cb *cb, void *data);

/* frequency = (422.4 + (CH_NO / 10)) * (1 + HFREQ_PLL)MHz */
void nrf905_set_channel_config(struct nrf905_ctx_t *ctx, uint8_t ch_no,
	enum nrf905_hfreq_pll_t hfreq_pll, enum nrf905_pa_pwr_t pa_pwr, spi_cb *cb);

/* WARNING writes the entire config */
void nrf905_save_config(struct nrf905_ctx_t *ctx, spi_cb *cb);

/* WARNING reads the entire config */
void nrf905_load_config(struct nrf905_ctx_t *ctx, spi_cb *cb);

#endif