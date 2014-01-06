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

#define CLK_SCALING 50
#define BUFFER_SIZE 4*1024

static uint8_t buffer[BUFFER_SIZE];

static const uint8_t PROTOCOL_VERSION[4] = { '1', 'A', 'L', 'S' };

static volatile uint32_t buf_pos;

static uint8_t METADATA[] = {
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
	uint32_t divider;
	uint16_t read_count;
	uint16_t delay_count;
	uint8_t trigger;
	uint8_t reset_count;
} ctx = {
	.divider = 50, /* run at full speed by default */
	.read_count = 0,
	.delay_count = 0,
	.trigger = 0,
	.reset_count = 0
};

static uint32_t
read_uint32(uint8_t n, uint8_t* data)
{
	uint32_t r = 0;
	while (n-- > 0) {
		r |= (data[n] << (n * 8)) & 0xff;
	}
	return r;
}

// static void
// dma_handler(uint8_t ch, uint32_t err, uint8_t major)
// {
// 	if (err) {
// 		onboard_led(ONBOARD_LED_OFF);
// 		blink(3);
// 	}
// 	if (++buf_pos == BUFFER_SIZE) {
// 		 we reached the end of the buffer, stop the dma channel and the timer
// 		dma_cancel(DMA_CH_0);
// 		pit_stop(PIT_0);
// 		onboard_led(ONBOARD_LED_OFF);
// 		/* reset buf_pos, we reuse it for sending data */
// 		buf_pos = ctx.write(buffer, BUFFER_SIZE);
// 	}
// }

static void
sump_reset()
{
	ctx.divider = 50; /* run at full speed by default */
	ctx.read_count = 0;
	ctx.delay_count = 0;
	ctx.trigger = 0;
	ctx.reset_count = 0;
	buf_pos = 0;
}

static void
pit_handler(enum pit_id id)
{
	buffer[buf_pos++] = GPIOD.pdir;
	if (buf_pos >= BUFFER_SIZE) {
		pit_stop(PIT_0);
		onboard_led(ONBOARD_LED_OFF);
		buf_pos = ctx.write(buffer, BUFFER_SIZE);
	}
}

static void
sump_arm()
{
	buf_pos = 0;
	onboard_led(ONBOARD_LED_ON);

	/* setup timer, cycles = clock rate divider * (sysclock / max sample rate) - 1 */
	pit_start(PIT_0, 50 - 1, pit_handler);
	// pit_start(PIT_0, (ctx.divider * CLK_SCALING) - 1, NULL);

	/* dma is set to a "always on" source because we will fire it via timer */
	// dma_start(DMA_CH_0, DMA_MUX_SRC_ALWAYS0, 1, dma_handler);
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
	dma_init();

	/* set the dma to read the GPIOD 1x 8bits (byte) and
	   move back 1 byte after reading (-1 address adjustment) */
	// dma_from(DMA_CH_0, (void*)&GPIOD, 1, DMA_TRANSFER_SIZE_8_BIT, 0, 0);
	// dma_from_addr_adj(DMA_CH_0, -1);
	/* set the dma to write to our buffer, 1 byte at a time.
	   we let the pointer move forward by 1 byte (no address adjustment). */
	// dma_to(DMA_CH_0, &buffer, 1, DMA_TRANSFER_SIZE_8_BIT, 0, 0);
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
		if (!ctx.trigger_cfg.serial) {
		}
		break;
	case SUMP_CMD_SET_TRIGGER_VALUE:
		ctx.trigger = data[0];
		break;
	case SUMP_CMD_SET_TRIGGER_CFG:
		ctx.trigger_cfg.raw = read_uint32(4, data);
		break;
	case SUMP_CMD_SET_DIVIDER:
		ctx.divider = read_uint32(3, data); // read 3 bytes
		break;
	case SUMP_CMD_SET_READ_DELAY_COUNT:
		ctx.read_count = read_uint32(2, data); // read first 2 bytes
		data += 2;
		ctx.delay_count = read_uint32(2, data); // read next 2 bytes
		break;
	case SUMP_CMD_SET_FLAGS:
		ctx.flags.raw = data[0];
		/* fallthrough */
	default:
		break;
	}
}
