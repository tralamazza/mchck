#include <mchck.h>

static struct timeout_ctx t;

/*
static void
blink(void *data)
{
	onboard_led(ONBOARD_LED_TOGGLE);
	timeout_add(&t, 500, blink, NULL);
}
*/

static void
blink_sync(void *data)
{
	*(int*)data = 1;
}


int
main(void)
{
	timeout_init();
	/* blink will also setup a timer to itself */
	// blink(NULL);

	for (;;) {
		onboard_led(ONBOARD_LED_TOGGLE);
		volatile int sem = 0;
		timeout_add(&t, 500, blink_sync, (void*)&sem);
		while (!sem)
			__asm__("wfi");
	}
	sys_yield_for_frogs();
}
