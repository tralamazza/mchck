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

enum hid_report_type {
	USB_HID_REPORT_TYPE_INPUT = 0x01,
	USB_HID_REPORT_TYPE_OUTPUT = 0x02,
	USB_HID_REPORT_TYPE_FEATURE = 0x03
};

/*
 * Handle class (HID) specific calls.
 *
 * see hid_handle_control()
 */
static int
hid_handle_control_class(struct usb_ctrl_req_t *req, struct hid_ctx *ctx)
{
	switch ((enum hid_ctrl_req_code)req->bRequest) {
	case USB_CTRL_REQ_HID_GET_REPORT:
		switch ((enum hid_report_type)req->wValueHigh) {
		case USB_HID_REPORT_TYPE_INPUT:
			// uint8_t buff[HID_TX_SIZE] = {}
			// ctx->report_input(req->wValueLow, &buf, HID_TX_SIZE);

		case USB_HID_REPORT_TYPE_OUTPUT:

		case USB_HID_REPORT_TYPE_FEATURE:
		default:
			return (0);
		}
	case USB_CTRL_REQ_HID_GET_IDLE:
		printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ GET IDLE");
		usb_ep0_tx_cp(&ctx->idle_rate, sizeof(uint8_t), req->wLength, NULL, NULL);
		usb_handle_control_status(0);
		return (1);
	// case USB_CTRL_REQ_HID_GET_PROTOCOL:
	// case USB_CTRL_REQ_HID_SET_REPORT:
	case USB_CTRL_REQ_HID_SET_IDLE:
		printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ SET IDLE");
		// req->wValueHigh // duration in 4ms steps (4..1020), 0 = infinity
		// req->wValueLow // Report ID, 0 = all reports
		if (req->wValueLow == 0)
			ctx->idle_rate = req->wValueHigh;
		return (1);
	// case USB_CTRL_REQ_HID_SET_PROTOCOL:
	default:
		return (0);
	}
}

/*
 * Handle non standard and non device calls.
 *
 * return non-zero if the call was handled.
 */
static int
hid_handle_control(struct usb_ctrl_req_t *req, void *data)
{
	struct hid_ctx *ctx = data;

	if (req->type == USB_CTRL_REQ_CLASS)
		return (hid_handle_control_class(req, ctx));

	switch (req->bRequest) {
	case USB_CTRL_REQ_GET_DESCRIPTOR:
		printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ GET DESCRIPTOR");
		usb_ep0_tx_cp(ctx->report, ctx->report_size, req->wLength, NULL, NULL);
		usb_handle_control_status(0);
		return (1);
	default:
		return (0);
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
	ctx->report_input = hid_desc->report_input;
	ctx->report_output = hid_desc->report_output;
	ctx->idle_rate = 0;
	// ctx->rx_pipe = usb_init_ep(&ctx->header, CDC_TX_EP, USB_EP_TX, HID_TX_SIZE);
	// ctx->tx_pipe = hid_desc->report_output ? usb_init_ep(&ctx->header, CDC_TX_EP, USB_EP_TX, 64) : NULL;
	usb_attach_function(&hid_function, &ctx->header);
}
