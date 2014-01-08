#include <stdio.h>
#include <mchck.h>

#include "mclogic.desc.h"

#define BUFFER_SIZE 4*1024

struct divider_t {
	uint32_t value : 24;
	uint32_t _pad : 8;
};

static struct cdc_ctx cdc;

static uint8_t buffer[BUFFER_SIZE];
static uint32_t buf_pos = 0;

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

static
struct sump_flags_t {
	UNION_STRUCT_START(8);
	uint8_t demux : 1;
	uint8_t filter : 1;
	uint8_t group0 : 1;
	uint8_t group1 : 1;
	uint8_t group2 : 1;
	uint8_t group3 : 1;
	uint8_t external : 1;
	uint8_t inverted : 1;
	UNION_STRUCT_END;
} flags;

static void
dump_data(uint8_t *data, size_t len)
{
	while (len-- > 0) {
		fprintf(stderr, "0x%x\t", *data++);
	}
	fprintf(stderr, "\n");
}

static void
data_sent(size_t value)
{
	if (buf_pos == 0)
		return;
	if ((value == CDC_TX_SIZE) && (buf_pos < BUFFER_SIZE)) {
		buf_pos += cdc_write(buffer + buf_pos, BUFFER_SIZE, &cdc);
		if (buf_pos >= BUFFER_SIZE) {
			fprintf(stderr, "reply finished\n");
		}
	}
}

static void
new_data(uint8_t *data, size_t len)
{
	dump_data(data, len);
	uint8_t cmd = *data++;
	switch (cmd) {
	case 0x1:
		fprintf(stderr, "sampling started\n");
		buf_pos = 0;
		while (buf_pos < BUFFER_SIZE) {
			buffer[buf_pos++] = buf_pos << 1;
		}
		fprintf(stderr, "sampling finished\n");
		buf_pos = cdc_write(buffer, BUFFER_SIZE, &cdc);
		break;
	case 0x2:
		cdc_write("1ALS", 4, &cdc);
		break;
	case 0x4:
		cdc_write(METADATA, sizeof(METADATA), &cdc);
		break;
	case 0x80:
		fprintf(stderr, "div=%u\n", ((struct divider_t*)data)->value);
		break;
	case 0x81:
		fprintf(stderr, "rc=%u, dc=%u\n", *(uint16_t*)data, *(uint16_t*)&data[2]);
		break;
	case 0x82:
		flags.raw = data[0];
		fprintf(stderr, "filter=0x%x, group0dis=0x%x, external=0x%x\n", flags.filter, flags.group0, flags.external);
		break;
	}
	cdc_read_more(&cdc);
}

void
init_vcdc(int config)
{
	cdc_init(new_data, data_sent, &cdc);
}

int
main(void)
{
	usb_init(&cdc_device);
	vusb_main_loop();
}
