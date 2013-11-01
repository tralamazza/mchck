// Required fwd declaration:
//   void init_my_hid_joy(inf config)

struct hid_desc_t {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdHID;
	uint8_t bCountryCode;
	uint8_t bNumDescriptors;
	uint8_t bDescriptorType1;
	uint16_t wDescriptorLength;
} __packed;

struct hid_function_desc {
	struct usb_desc_iface_t iface;
	struct hid_desc_t hid_desc;
	struct usb_desc_ep_t int_in_ep;
} __packed;

struct usb_config_hid {
	struct usb_desc_config_t config;
	struct hid_function_desc usb_function_0;
} __packed;

#define USB_INTERFACE_CLASS_HID 0x3
#define USB_INTERFACE_SUBCLASS_NOCLASS 0x0
#define USB_INTERFACE_SUBCLASS_BOOT 0x1
#define USB_INTERFACE_PROTOCL_HID_NONE 0x0
#define USB_INTERFACE_PROTOCL_HID_KEYBOARD 0x1
#define USB_INTERFACE_PROTOCL_HID_MOUSE 0x2

#define JOYSTICK_TX_SIZE 16

#define REPORT_DESC_SIZE 46
// http://www.instructables.com/id/USB-Wii-Classic-Controller/step5/USB-HID-Reports-and-Report-Descriptors/
static uint8_t report_desc[REPORT_DESC_SIZE] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x05,                    // USAGE (Game Pad)
    0xa1, 0x01,                    // COLLECTION (Application)
    0xa1, 0x00,                    //   COLLECTION (Physical)
    0x05, 0x09,                    //     USAGE_PAGE (Button)
    0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
    0x29, 0x10,                    //     USAGE_MAXIMUM (Button 16)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x95, 0x10,                    //     REPORT_COUNT (16)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x09, 0x32,                    //     USAGE (Z)
    0x09, 0x33,                    //     USAGE (Rx)
    0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
    0x25, 0x7f,                    //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x04,                    //     REPORT_COUNT (4)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0xc0,                          //   END_COLLECTION
    0xc0                           // END_COLLECTION
};

struct joystick_data_t {
	union {
		struct {
			uint8_t square_btn : 1;
			uint8_t cross_btn : 1;
			uint8_t circle_btn : 1;
			uint8_t triangle_btn : 1;
			uint8_t l1_btn : 1;
			uint8_t r1_btn : 1;
			uint8_t l2_btn : 1;
			uint8_t r2_btn : 1;
			uint8_t select_btn : 1;
			uint8_t start_btn : 1;
			uint8_t l3_btn : 1;
			uint8_t r3_btn : 1;
			uint8_t ps_btn : 1;
			uint8_t pad: 3;
		};
		uint16_t buttons;
	};
	int8_t left_x;
	int8_t left_y;
	int8_t right_x;
	int8_t right_y;
} __packed;

static struct joystick_data_t joystick_data = {
	.buttons = 0,
	.left_x = 0,
	.left_y = 0,
	.right_x = 0,
	.right_y = 0
};

static const struct usb_config_hid hid_desc_config = {
	.config = {
		.bLength = sizeof(struct usb_desc_config_t),
		.bDescriptorType = USB_DESC_CONFIG,
		.wTotalLength = sizeof(struct usb_config_hid),
		.bNumInterfaces = 1,
		.bConfigurationValue = 1,
		.iConfiguration = 0,
		.one = 1,
		.bMaxPower = 50
	},
	.usb_function_0 = {
		.iface = {
			.bLength = sizeof(struct usb_desc_iface_t),
			.bDescriptorType = 0x4,
			.bInterfaceNumber = 0x0,
			.bAlternateSetting = 0x0,
			.bNumEndpoints = 0x1,
			.bInterfaceClass = USB_INTERFACE_CLASS_HID,
			.bInterfaceSubClass = USB_INTERFACE_SUBCLASS_NOCLASS,
			.bInterfaceProtocol = USB_INTERFACE_PROTOCL_HID_NONE,
			.iInterface = 0x0
		},
		.hid_desc = {
			.bLength = 0x9,
			.bDescriptorType = 0x21,
			.bcdHID = 0x101,
			.bCountryCode = 0x0,
			.bNumDescriptors = 0x1,
			.bDescriptorType1 = 0x22,
			.wDescriptorLength = REPORT_DESC_SIZE,
		},
		.int_in_ep = {
			.bLength = sizeof(struct usb_desc_ep_t),
			.bDescriptorType = USB_DESC_EP,
			.ep_num = 1,
			.in = 1,
			.type = USB_EP_INTR,
			.wMaxPacketSize = JOYSTICK_TX_SIZE,
			.bInterval = 0xa
		}
	}
};

static const struct usbd_function hid_usbd_function = {
};

static const struct usbd_config usbd_hid_config = {
	.init = init_my_hid_joy,
	.desc = &hid_desc_config.config,
	.function = { &hid_usbd_function }
};

static const struct usb_desc_string_t * const hid_usb_strings[] = {
	USB_DESC_STRING_LANG_ENUS,
	USB_DESC_STRING(u"mchck"),
	USB_DESC_STRING(u"mchck joystick"),
	USB_DESC_STRING_SERIALNO,
	NULL
};

static const struct usb_desc_dev_t
hid_usb_dev_desc = {
	.bLength = sizeof(struct usb_desc_dev_t),
	.bDescriptorType = USB_DESC_DEV,
	.bcdUSB = { .maj = 2 },
	.bDeviceClass = 0,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = EP0_BUFSIZE,
	.idVendor = 8995,
	.idProduct = 3,
	.bcdDevice = { .raw = 0 },
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1,
};

static const struct usbd_device hid_dev = {
	.dev_desc = &hid_usb_dev_desc,
	.string_descs = hid_usb_strings,
	.configs = {
		&usbd_hid_config,
		NULL
	}
};
