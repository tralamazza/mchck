#include "sump.h"


enum sump_cmd_t {
	/* short commands */
	SUMP_CMD_RESET =                0x0,
	SUMP_CMD_ARM =                  0x1,
	SUMP_CMD_ID =                   0x2,
	SUMP_CMD_SELF_TEST =            0x3, // extended
	SUMP_CMD_GET_METADATA =         0x4, // extended
	SUMP_CMD_RLE_END =              0x5, // extended
	SUMP_CMD_XON =                  0x11,
	SUMP_CMD_XOFF =                 0x13,
	/* long commands */
	SUMP_CMD_SET_TRIGGER_MASK =     0xc0,
	SUMP_CMD_SET_TRIGGER_VALUE =    0xc1,
	SUMP_CMD_SET_TRIGGER_CFG =      0xc2,
	SUMP_CMD_SET_DIVIDER =          0x80,
	SUMP_CMD_SET_READ_DELAY_COUNT = 0x81,
	SUMP_CMD_SET_FLAGS =            0x82
};

#define NUM_PROBES 8
#define CLK_SCALING 50
#define BUFFER_SIZE 4*1024

static uint8_t buffer[BUFFER_SIZE];

static volatile uint32_t buf_pos;

static const uint8_t PROTOCOL_VERSION[4] = { '1', 'A', 'L', 'S' };

static const uint8_t METADATA[] = {
	0x01, 'M', 'C', 'H', 'C', 'K', 0x00, // device name
	0x02, '0', '.', '1', 0x00, // firmware version
	// 0x20, 0x00, 0x00, 0x00, 0x08, // 8 probes (long)
	0x21, 0x00, 0x00, 0x10, 0x00, // 4096 bytes of memory (BUFFER_SIZE)
	0x23, 0x00, 0x0f, 0x42, 0x40, // max sample rate (1MHz)
	0x40, 0x08, // 8 probes (short)
	0x41, 0x02, // protocol 2
	0x00
};

struct sump_flags_t {
	UNION_STRUCT_START(8);
	uint8_t demux : 1;
	uint8_t filter : 1;
	uint8_t group0_dis : 1;
	uint8_t group1_dis : 1;
	uint8_t group2_dis : 1;
	uint8_t group3_dis : 1;
	uint8_t external : 1;
	uint8_t inverted : 1;
	UNION_STRUCT_END;
};

struct sump_trigger_config_t {
	UNION_STRUCT_START(32);
	uint32_t channel4 : 1;
	uint32_t _pad0 : 1;
	uint32_t serial : 1;
	uint32_t start : 1;
	uint32_t _pad1 : 4;
	uint32_t level : 2;
	uint32_t _pad2 : 2;
	uint32_t channel03 : 4;
	uint16_t delay;
	UNION_STRUCT_END;
};

static volatile
struct sump_context {
	sump_writer *write;
	struct sump_flags_t flags;
	struct sump_trigger_config_t trigger_cfg;
	uint32_t trigger;
	uint32_t trigger_mask;
	uint32_t divider;
	uint16_t read_count;
	uint16_t delay_count;
	uint8_t reset_count;
} ctx = {
	.trigger = 0,
	.trigger_mask = 0,
	.divider = 1, /* run at full speed by default */
	.read_count = 0,
	.delay_count = 0,
	.reset_count = 0
};

struct divider_t {
	uint32_t value : 24;
	uint32_t _pad : 8;
};

static void
sump_reset()
{
	ctx.trigger = 0;
	ctx.trigger_mask = 0;
	ctx.divider = 1; /* run at full speed by default */
	ctx.read_count = 0;
	ctx.delay_count = 0;
	ctx.reset_count = 0;
	buf_pos = 0;
	pit_stop(PIT_0); /* if anything is running, kill it */
	onboard_led(ONBOARD_LED_OFF);
}

#ifdef MCLOGIC_DMA

/* the dma samples and stores in buffer, this handler just checks for termination */
static void
dma_handler(uint8_t ch, uint32_t err, uint8_t major)
{
	if ((++buf_pos >= BUFFER_SIZE) || err) {
		onboard_led(ONBOARD_LED_OFF);
		dma_cancel(DMA_CH_0);
		pit_stop(PIT_0);
		buf_pos = ctx.write(buffer, BUFFER_SIZE);
	}
}
#else

/* timer handler will sample the port and store in buffer */
static void
pit_handler_sample(enum pit_id id)
{
	buffer[buf_pos++] = GPIOD.pdir;
	if (buf_pos >= BUFFER_SIZE) {
		onboard_led(ONBOARD_LED_OFF);
		pit_stop(PIT_0);
		buf_pos = ctx.write(buffer, BUFFER_SIZE);
	}
}
#endif

static void
start_sampling()
{
	buf_pos = 0;
	/* setup timer, cycles = clock rate divider * (sysclock / max sample rate) - 1 */
#ifdef MCLOGIC_DMA
	/* set the dma to write to our buffer, 1 byte at a time.
	   we let the pointer move forward by 1 byte (no address adjustment). */
	dma_to(DMA_CH_0, buffer, 1, DMA_TRANSFER_SIZE_8_BIT, 0, 0);
	dma_to_addr_adj(DMA_CH_0, 0);
	/* configure the timer according to our divider and clk. handler is not required. */
	pit_start(PIT_0, (ctx.divider * CLK_SCALING) - 1, NULL);
	/* dma has to be set to a "always on" source because PIT gates it */
	dma_start(DMA_CH_0, DMA_MUX_SRC_ALWAYS0, 1, dma_handler);
#else
	/* configure the timer according to our divider and clk */
	pit_start(PIT_0, (ctx.divider * CLK_SCALING) - 1, pit_handler_sample);
#endif
	onboard_led(ONBOARD_LED_ON);
}

/* handles triggering */
void
PORTD_Handler(void)
{
	int_disable(IRQ_PORTD);
	uint32_t i;
	for (i = 0; i < NUM_PROBES; i++) {
		pin_physport_from_pin(PIN_PTD0 + i)->pcr[pin_physpin_from_pin(PIN_PTD0 + i)].raw |= 0;
	}
	start_sampling();
}

static void
sump_arm()
{
	/* parallel trigger not supported */
	if (/*ctx.trigger_cfg.serial &&*/ ctx.trigger_mask != 0) {
		uint32_t mask, i;
		for (i = 0; i < NUM_PROBES; i++) {
			mask = i << 1; /* used to check if the port is used as trigger and its value */
			enum PCR_IRQC_t irqc;
			if (ctx.trigger_mask & mask) {
				irqc = ctx.trigger & mask ? PCR_IRQC_INT_ONE : PCR_IRQC_INT_ZERO;
			} else {
				irqc = PCR_IRQC_DISABLED;
			}
			/* PIN_PTD0 + i = assumes port addr are contiguous */
			pin_physport_from_pin(PIN_PTD0 + i)->pcr[pin_physpin_from_pin(PIN_PTD0 + i)].irqc = irqc;
		}
		int_enable(IRQ_PORTD);
	} else {
		start_sampling();
	}
}

void
sump_init(sump_writer *w)
{
	ctx.write = w;

	/* configure the entire PORTD as input*/
	gpio_dir(PIN_PTD0, GPIO_INPUT);
	gpio_dir(PIN_PTD1, GPIO_INPUT);
	gpio_dir(PIN_PTD2, GPIO_INPUT);
	gpio_dir(PIN_PTD3, GPIO_INPUT);
	gpio_dir(PIN_PTD4, GPIO_INPUT);
	gpio_dir(PIN_PTD5, GPIO_INPUT);
	gpio_dir(PIN_PTD6, GPIO_INPUT);
	gpio_dir(PIN_PTD7, GPIO_INPUT);

	pit_init();
#ifdef MCLOGIC_DMA
	dma_init();
	/* set the dma to read the GPIOD 1x 8bits (byte) and
	   move back 1 byte after reading (-1 address adjustment) */
	dma_from(DMA_CH_0, (void*)0x400ff0C0, 1, DMA_TRANSFER_SIZE_8_BIT, 0, 0);
#endif
}

void
sump_data_sent(size_t value)
{
	if (buf_pos == 0)
		return;
	if ((value == CDC_TX_SIZE) && (buf_pos < BUFFER_SIZE)) {
		buf_pos += ctx.write(buffer + buf_pos, BUFFER_SIZE);
	}
}

void
sump_process(uint8_t* data, size_t len)
{
	if (data[0] == SUMP_CMD_RESET) {
		if (++ctx.reset_count == 5) {
			return sump_reset(); // only after 5 consecutive reset cmds
		}
	} else {
		ctx.reset_count = 0;
	}

	switch(*data++) { // read and skip 1st byte
	case SUMP_CMD_ARM:
		sump_arm();
		break;
	case SUMP_CMD_ID:
		ctx.write(PROTOCOL_VERSION, 4);
		break;
	case SUMP_CMD_SELF_TEST:
		break;
	case SUMP_CMD_GET_METADATA:
		ctx.write(METADATA, sizeof(METADATA));
		break;
	case SUMP_CMD_RLE_END:
	case SUMP_CMD_XON:
	case SUMP_CMD_XOFF:
		break;
	case SUMP_CMD_SET_TRIGGER_MASK:
		ctx.trigger_mask = *(uint32_t*)data;
		break;
	case SUMP_CMD_SET_TRIGGER_VALUE:
		ctx.trigger = *(uint32_t*)data;
		break;
	case SUMP_CMD_SET_TRIGGER_CFG:
		ctx.trigger_cfg.raw = *(uint32_t*)data;
		break;
	case SUMP_CMD_SET_DIVIDER:
		ctx.divider = ((struct divider_t*)data)->value;
#ifdef MCLOGIC_SIGROK
		ctx.divider = (ctx.divider + 1) / 100;
#endif
		break;
	case SUMP_CMD_SET_READ_DELAY_COUNT:
		ctx.read_count = ((*(uint16_t*)data) + 1) * 4; // read first 2 bytes
		data += 2;
		ctx.delay_count = ((*(uint16_t*)data) + 1) * 4; // read next 2 bytes
		break;
	case SUMP_CMD_SET_FLAGS:
		ctx.flags.raw = data[0];
		/* fallthrough */
	default:
		break;
	}
}

