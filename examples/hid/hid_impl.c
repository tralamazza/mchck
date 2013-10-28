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

static int
hid_handle_control(struct usb_ctrl_req_t *req, void *data)
{
	printf("!!!!!!!!! ReqType %x | Req %x !!!!!!!!!\n", req->bmRequestType, req->bRequest);
	printf("!!!!!!!!! recp %x | type %x | in %x !!!!!!!!!\n", req->recp, req->type, req->in);
	struct hid_ctx *ctx = data;
	switch ((enum hid_ctrl_req_code)req->bRequest) {
	case USB_CTRL_REQ_HID_GET_REPORT:
		usb_ep0_tx_cp(&ctx->report, ctx->report_size, req->wLength, NULL, NULL);
		usb_handle_control_status(0);
		break;
		// USB_CTRL_REQ_HID_GET_IDLE
		// USB_CTRL_REQ_HID_GET_PROTOCOL
		// USB_CTRL_REQ_HID_SET_REPORT
	case USB_CTRL_REQ_HID_SET_IDLE:
		// printf("SET IDLE: REPORT ID %d, DURATION %d\n", req->wValueLow, req->wValueHigh);
		return (0);
		// USB_CTRL_REQ_HID_SET_PROTOCOL
	default:
		return (0);
	}
	return (1);
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
