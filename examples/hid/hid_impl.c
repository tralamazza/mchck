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

#define USB_FUNCTION_HID_IFACE_COUNT 1

/*
 * Handle class (HID) specific calls.
 *
 * see hid_handle_control()
 */
static int
hid_handle_control_class(struct usb_ctrl_req_t *req, struct hid_ctx *ctx)
{
	void *buf = NULL;
	size_t len = 0;

	switch ((enum hid_ctrl_req_code)req->bRequest) {
	case USB_CTRL_REQ_HID_GET_REPORT:
		if (!ctx->user_functions->get_report)
			return (0);
		len = ctx->user_functions->get_report(req->wValueHigh, req->wValueLow, &buf);
		if (!len)
			return (0);
		usb_ep0_tx_cp(buf, len, req->wLength, NULL, NULL);
		usb_handle_control_status(0);
		return (1);

	case USB_CTRL_REQ_HID_GET_IDLE:
		if (!ctx->user_functions->get_idle)
			return (0);
		len = ctx->user_functions->get_idle(req->wValueLow);
		usb_ep0_tx_cp(&len, 1, req->wLength, NULL, NULL);
		usb_handle_control_status(0);
		return (1);

	case USB_CTRL_REQ_HID_GET_PROTOCOL:
		if (!ctx->user_functions->get_protocol)
			return (0);
		len = ctx->user_functions->get_protocol();
		usb_ep0_tx_cp(&len, 1, req->wLength, NULL, NULL);
		usb_handle_control_status(0);
		return (1);

	case USB_CTRL_REQ_HID_SET_REPORT: // TODO
		return (0);

	case USB_CTRL_REQ_HID_SET_IDLE:
		if (!ctx->user_functions->set_idle)
			return (0);
		ctx->user_functions->set_idle(req->wValueHigh, req->wValueLow);
		return (1);

	case USB_CTRL_REQ_HID_SET_PROTOCOL:
		if (!ctx->user_functions->set_protocol)
			return (0);
		ctx->user_functions->set_protocol(req->wValue);
		return (1);

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
	void *buf = NULL;
	size_t len = 0;

	if (req->type == USB_CTRL_REQ_CLASS)
		return (hid_handle_control_class(req, ctx));

	switch (req->bRequest) {
	case USB_CTRL_REQ_GET_DESCRIPTOR:
		len = ctx->user_functions->get_descriptor(req->wValueHigh, req->wValueLow, &buf);
		if (!len)
			return (0);
		usb_ep0_tx_cp(buf, len, req->wLength, NULL, NULL);
		usb_handle_control_status(0);
		return (1);

	case USB_CTRL_REQ_SET_DESCRIPTOR: // TODO
		return (0);

	default:
		return (0);
	}
}

const struct usbd_function hid_function = {
	.control = hid_handle_control,
	.interface_count = USB_FUNCTION_HID_IFACE_COUNT
};

void
hid_init(struct hid_user_functions_t *user_functions, struct hid_ctx *ctx)
{
	usb_attach_function(&hid_function, &ctx->header);
	ctx->user_functions = user_functions;
	ctx->tx_pipe = usb_init_ep(&ctx->header, 1, USB_EP_TX, 8);
}
