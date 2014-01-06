#include "blink.h"

static struct timeout_ctx blink_timer;
static volatile uint32_t blink_count;
static uint32_t blink_period = 200;

static void
blink_handler(void *data)
{
	onboard_led(ONBOARD_LED_TOGGLE);
	if (--blink_count > 0)
		timeout_add(&blink_timer, blink_period, blink_handler, NULL);
}

void
blink_init(uint32_t period)
{
	timeout_init();
	blink_count = 0;
	blink_period = period == 0 ? 200 : period;
	onboard_led(ONBOARD_LED_OFF);
}

void
blink(uint32_t n)
{
	if (n == 0) return;
	if (blink_count == 0) {
		onboard_led(ONBOARD_LED_ON);
		timeout_add(&blink_timer, blink_period, blink_handler, NULL);
	}
	blink_count += (n * 2) - 1;
}
