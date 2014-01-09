PROG=	mclogic

ifdef DMA
CFLAGS+=	-DMCLOGIC_DMA
endif

ifndef VUSB
SRCS+=	sump.c
SRCS+=	main-kinetis.c
else
TARGET=	host
SRCS+=	vcdc.c
endif

SRCS+=	mclogic.desc

include ../../toolchain/mchck.mk
