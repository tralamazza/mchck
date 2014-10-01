#include <mchck.h>
#include "nrf905.h"

static struct nrf905_ctx_t ctx;
static struct timeout_ctx t;
static uint8_t channel = 0;

static void channel_changed(void *data);

static void
app_state_handler(void *data)
{
	nrf905_set_channel_config(&ctx, channel++, HFREQ_PLL_433MHZ, PA_PWR_10DBM, channel_changed);
}

static void
channel_changed(void *data)
{
	timeout_add(&t, 10, app_state_handler, NULL);
	nrf905_send(&ctx, &channel, 1, NULL);
}

NRF905_INT_DECL(&ctx);

int
main(void)
{
	timeout_init();
	spi_init();
	pin_change_init();
	nrf905_init(&ctx, app_state_handler);
	sys_yield_for_frogs();
}