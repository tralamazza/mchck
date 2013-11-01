PROG=	hid

ifndef VUSB
SRCS+=	hid_joy.c
else
TARGET=	host
SRCS+=	vhid.c
endif
SRCS+=	hid_impl.c

include ../../toolchain/mchck.mk
