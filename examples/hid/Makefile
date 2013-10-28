PROG=	hid

ifndef VUSB
SRCS+=	khid.c
else
TARGET=	host
SRCS+=	vhid.c
endif
SRCS+=  hid_impl.c

include ../../toolchain/mchck.mk
