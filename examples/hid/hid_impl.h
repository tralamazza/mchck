#ifndef _USB_HID_H
#define _USB_HID_H

#define USB_FUNCTION_HID_IFACE_COUNT 1

#define HID_TX_SIZE 64

typedef int (*hid_report_input_t)(uint8_t report_id, void *data_out, size_t data_size);
typedef int (*hid_report_output_t)(uint8_t report_id, void *data_in, size_t data_size);

struct hid_descriptor_t {
	uint16_t report_size;
	uint8_t *report;
	hid_report_input_t report_input;
	hid_report_output_t report_output;
};

struct hid_ctx {
	/* inputs */
	uint16_t report_size;
	uint8_t *report;
	hid_report_input_t report_input;
	hid_report_output_t report_output;
	/* state */
	struct usbd_function_ctx_header header;
	uint8_t idle_rate;
	struct usbd_ep_pipe_state_t *tx_pipe;
	struct usbd_ep_pipe_state_t *rx_pipe;
};

void hid_init(const struct hid_descriptor_t *hid_desc, struct hid_ctx *ctx);

#endif
