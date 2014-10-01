#include <mchck.h>
#include "nrf905.h"
#include "nrf905_usb.desc.h"

#define PAYLOAD_LEN 32

static struct cdc_ctx cdc;
static struct nrf905_ctx_t ctx;
static struct timeout_ctx t;
static uint16_t channel = 0;
static uint8_t payload[PAYLOAD_LEN];

static void channel_changed(void *data);

static const uint16_t MAX_CH_NO = (1 << 10) - 1;

static void
change_channel(void *data)
{
	channel = channel % MAX_CH_NO;
	channel++;
	nrf905_set_channel_config(&ctx, channel, HFREQ_PLL_433MHZ, PA_PWR_10DBM, channel_changed);
}

static void
save_config(void *data)
{
	ctx.config.RX_AFW = 0; // zero RX addr
	ctx.config.CRC_EN = 0; // no CRC
	nrf905_save_config(&ctx, change_channel);
}

static void
recv_done(void *data, uint8_t len)
{
	onboard_led(ONBOARD_LED_TOGGLE);
	printf("%d\t%32s\r\n", channel, (char*)data);
}

static void
channel_changed(void *data)
{
	timeout_add(&t, 50, change_channel, NULL);
	nrf905_receive(&ctx, payload, PAYLOAD_LEN, recv_done);
}

NRF905_INT_DECL(&ctx);

void
init_vcdc(int config)
{
        cdc_init(NULL, NULL, &cdc);
        cdc_set_stdout(&cdc);
}

int
main(void)
{
	timeout_init();
	spi_init();
	pin_change_init();
	usb_init(&cdc_device);
	nrf905_init(&ctx, save_config);
	sys_yield_for_frogs();
}