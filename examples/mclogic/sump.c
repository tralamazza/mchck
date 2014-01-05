#include "sump.h"

enum sump_cmd_t {
	// short commands
	SUMP_CMD_RESET = 0x0,
	SUMP_CMD_ARM = 0x1,
	SUMP_CMD_ID = 0x2,
	SUMP_CMD_SELF_TEST = 0x3, // extended
	SUMP_CMD_GET_METADATA = 0x4, // extended
	SUMP_CMD_RLE_END = 0x5, // extended
	SUMP_CMD_XON = 0x11,
	SUMP_CMD_XOFF = 0x13,
	// long commands
	SUMP_CMD_SET_TRIGGER_MASK = 0xc0,
	SUMP_CMD_SET_TRIGGER_VALUE = 0xc1,
	SUMP_CMD_SET_TRIGGER_CFG = 0xc2,
	SUMP_CMD_SET_DIVIDER = 0x80,
	SUMP_CMD_SET_READ_DELAY_COUNT = 0x81,
	SUMP_CMD_SET_FLAGS = 0x82
};

static const uint8_t PROTOCOL_VERSION[4] = { '1', 'A', 'L', 'S' };

#define BUFFER_SIZE 7*1024

static uint8_t METADATA[] = {
	0x1, 'M', 'C', 'H', 'C', 'K', 0x0, // device name
	0x2, '0', '.', '1', 0x0, // firmware version
	0x21, 0x0, 0x0, 0x1c, 0x0, // memory 7168
	0x23, 0x0, 0x3d, 0x9, 0x0, // max sample rate (4MHz)
	0x40, 0x8, // probes
	0x41, 0x2, // protocol
	0x0
};

struct sump_flags_t {
	union {
		uint8_t inverted : 1;
		uint8_t external : 1;
		uint8_t groups : 4;
		uint8_t filter : 1;
		uint8_t demux : 1;
		uint8_t value;
	};
};

struct sump_context {
	sump_writer *write;
	struct sump_flags_t flags;
} ctx;

static void
sump_arm()
{
	//
}

void
sump_init(sump_writer *w)
{
	ctx.write = w;
	gpio_dir(PIN_PTD0, GPIO_INPUT);
	gpio_dir(PIN_PTD1, GPIO_INPUT);
	gpio_dir(PIN_PTD2, GPIO_INPUT);
	gpio_dir(PIN_PTD3, GPIO_INPUT);
	gpio_dir(PIN_PTD4, GPIO_INPUT);
	gpio_dir(PIN_PTD5, GPIO_INPUT);
	gpio_dir(PIN_PTD6, GPIO_INPUT);
	gpio_dir(PIN_PTD7, GPIO_INPUT);

	dma_init();
}

void
sump_process(uint8_t* data, size_t len)
{
	switch(data[0]) {
	case SUMP_CMD_RESET:
		// should test first 5 bytes in data for 0x0
		break;
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
	case SUMP_CMD_SET_TRIGGER_MASK:
	case SUMP_CMD_SET_TRIGGER_VALUE:
	case SUMP_CMD_SET_TRIGGER_CFG:
	case SUMP_CMD_SET_DIVIDER:
	case SUMP_CMD_SET_READ_DELAY_COUNT:
		break;
	case SUMP_CMD_SET_FLAGS:
		ctx.flags.value = data[1];
		break;
	default:
		break;
	}
}

