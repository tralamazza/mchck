PROG=	mclogic

SRCS+=	sump.c

ifndef VUSB
SRCS+=	main-kinetis.c
else
TARGET=	host
SRCS+=	vcdc.c
endif

SRCS+=	mclogic.desc

include ../../toolchain/mchck.mk
