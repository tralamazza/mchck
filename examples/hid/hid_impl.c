#include <usb/usb.h>

#include "hid_impl.h"

enum hid_ctrl_req_code {
	USB_CTRL_REQ_HID_GET_REPORT = 0x01,
	USB_CTRL_REQ_HID_GET_IDLE = 0x02,
	USB_CTRL_REQ_HID_GET_PROTOCOL = 0x03,
	USB_CTRL_REQ_HID_SET_REPORT = 0x09,
	USB_CTRL_REQ_HID_SET_IDLE = 0x0A,
	USB_CTRL_REQ_HID_SET_PROTOCOL = 0x0B
};

/*enum hid_report_type {
	USB_HID_REPORT_TYPE_INPUT = 0x01,
	USB_HID_REPORT_TYPE_OUTPUT = 0x02,
	USB_HID_REPORT_TYPE_FEATURE = 0x03
};*/

static int
hid_handle_control_class(struct usb_ctrl_req_t *req, struct hid_ctx *ctx)
{
	switch ((enum hid_ctrl_req_code)req->bRequest) {
	// case USB_CTRL_REQ_HID_GET_REPORT:
	// case USB_CTRL_REQ_HID_GET_IDLE:
	// case USB_CTRL_REQ_HID_GET_PROTOCOL:
	// case USB_CTRL_REQ_HID_SET_REPORT:
	// case USB_CTRL_REQ_HID_SET_IDLE:
	// case USB_CTRL_REQ_HID_SET_PROTOCOL:
	default:
		return (0);
	}
}

static int
hid_handle_control(struct usb_ctrl_req_t *req, void *data)
{
	struct hid_ctx *ctx = data;

	if (req->type == USB_CTRL_REQ_CLASS)
		return (hid_handle_control_class(req, ctx));

	switch (req->bRequest) {
	case USB_CTRL_REQ_GET_DESCRIPTOR:
		usb_ep0_tx_cp(ctx->report, ctx->report_size, req->wLength, NULL, NULL);
		usb_handle_control_status(0);
		return (0);
	default:
		return (1);
	}
}

const struct usbd_function hid_function = {
	.control = hid_handle_control,
	.interface_count = USB_FUNCTION_HID_IFACE_COUNT
};

void hid_init(const struct hid_descriptor_t *hid_desc, struct hid_ctx *ctx)
{
	ctx->report_size = hid_desc->report_size;
	ctx->report = hid_desc->report;
	usb_attach_function(&hid_function, &ctx->header);
}
