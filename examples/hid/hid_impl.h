#ifndef _USB_HID_H
#define _USB_HID_H

#define USB_FUNCTION_HID_IFACE_COUNT 1

struct hid_descriptor_t {
	uint16_t report_size;
	uint8_t *report;
};

struct hid_ctx {
	struct usbd_function_ctx_header header;
	uint16_t report_size;
	uint8_t *report;
};

void hid_init(const struct hid_descriptor_t *hid_desc, struct hid_ctx *ctx);

#endif
