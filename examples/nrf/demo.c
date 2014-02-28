#include <mchck.h>
#include <nrf/nrf.h>

#include "demo.desc.h"

#define BUF_SIZE 32


static struct cdc_ctx cdc;
static uint64_t recv_addr = 0xf0f0f0f0e1ll;
static uint64_t send_addr = 0xf0f0f0f0f2ll;
static struct nrf_addr_t addr = { .value = 0, .size = 5 };
static uint8_t data_buffer[BUF_SIZE];
static int cmd_pos = 0;
static uint8_t cmd[16];

static void
data_sent(void *data, uint8_t len)
{
	if (data) {
		printf("send OK\r\n");
	} else {
		printf("send failed\r\n");
	}
	cdc_read_more(&cdc);
}

static void
data_received(void *data, uint8_t len)
{
	if (data) {
		cdc_write(data, len, &cdc);
	} else {
		printf("recv failed\r\n");
	}
	cdc_read_more(&cdc);
}

static void
demo_config(char* data, uint8_t len)
{
	switch (*data++) {
	case 'a':
		switch (*data++) {
		case 'r':
			memcpy(&recv_addr, data, 5);
			printf("recv addr %llx\r\n", recv_addr);
			break;
		case 's':
			memcpy(&send_addr, data, 5);
			printf("send addr %llx\r\n", send_addr);
			break;
		}
		break;
	case 'c': {
		uint8_t ch = atoi(data);
		nrf_set_channel(ch);
		printf("channel %d\r\n", ch);
		break;
	};
	case 'l':
		switch (*data) {
		case '1':
			nrf_set_crc_length(NRF_CRC_ENC_1_BYTE);
			printf("1 byte crc\r\n");
			break;
		case '2':
			nrf_set_crc_length(NRF_CRC_ENC_2_BYTES);
			printf("2 bytes crc\r\n");
			break;
		}
		break;
	case 'p':
		if (*data == '1') {
			nrf_enable_powersave();
			printf("powersave ON\r\n");
		} else {
			nrf_disable_powersave();
			printf("powersave OFF\r\n");
		}
		break;
	}
}

static void
cdc_new_data(uint8_t *data, size_t len)
{
	cdc_write(data, len, &cdc);

	size_t tmp = len;
	while (tmp-- > 0 && *data != '\r') {
		cmd[cmd_pos++] = *data++;
	}
	if (*data != '\r') {
		cdc_read_more(&cdc);
		return;
	}

	tmp = cmd_pos - 1;
	switch (cmd[0]) {
	case 'c':
		demo_config((char*)&cmd[1], tmp);
		cdc_read_more(&cdc);
		break;
	case 'r':
		addr.value = recv_addr;
		nrf_receive(&addr, data_buffer, BUF_SIZE, data_received);
		break;
	case 's':
		addr.value = send_addr;
		memcpy(data_buffer, &cmd[1], tmp);
		nrf_send(&addr, data_buffer, tmp, data_sent);
		break;
	}
	cmd_pos = 0;
}

void
init_vcdc(int enable)
{
	if (enable) {
		cdc_init(cdc_new_data, NULL, &cdc);
		cdc_set_stdout(&cdc);
	}
}

int
main(void)
{
	nrf_init();
	nrf_enable_dynamic_payload();
	usb_init(&cdc_device);
	sys_yield_for_frogs();
}
