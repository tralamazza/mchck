enum dma_transfer_size_t {
	DMA_TRANSFER_SIZE_8_BIT    = 0x0,
	DMA_TRANSFER_SIZE_16_BIT   = 0x1,
	DMA_TRANSFER_SIZE_32_BIT   = 0x2,
	DMA_TRANSFER_SIZE_16_BYTES = 0x4,
	DMA_TRANSFER_SIZE_32_BYTES = 0x5
};

struct DMA_t {
	struct {
		UNION_STRUCT_START(32);
		uint32_t _res0 : 1;
		uint32_t edbg  : 1;
		uint32_t erca  : 1;
		uint32_t _res1 : 1;
		uint32_t hoe   : 1;
		uint32_t halt  : 1;
		uint32_t clm   : 1;
		uint32_t emlm  : 1;
		uint32_t _res2 : 8;
		uint32_t ecx   : 1;
		uint32_t cx    : 1;
		uint32_t _res3 : 14;
		UNION_STRUCT_END;
	} cr;

	struct {
		UNION_STRUCT_START(32);
		uint32_t dbe    : 1;
		uint32_t sbe    : 1;
		uint32_t sge    : 1;
		uint32_t nce    : 1;
		uint32_t doe    : 1;
		uint32_t dae    : 1;
		uint32_t soe    : 1;
		uint32_t sae    : 1;
		uint32_t errchn : 2;
		uint32_t _res0  : 4;
		uint32_t cpe    : 1;
		uint32_t _res1  : 1;
		uint32_t ecx    : 1;
		uint32_t _res2  : 14;
		uint32_t vld    : 1;
		UNION_STRUCT_END;
	} es;

	uint32_t _pad0;

	struct {
		UNION_STRUCT_START(32);
		uint32_t erq0 : 1;
		uint32_t erq1 : 1;
		uint32_t erq2 : 1;
		uint32_t erq3 : 1;
		uint32_t _res : 28;
		UNION_STRUCT_END;
	} erq;

	uint32_t _pad1;

	struct {
		UNION_STRUCT_START(32);
		uint32_t eei0 : 1;
		uint32_t eei1 : 1;
		uint32_t eei2 : 1;
		uint32_t eei3 : 1;
		uint32_t _res : 28;
		UNION_STRUCT_END;
	} eei;

	struct {
		UNION_STRUCT_START(8);
		uint8_t ceei : 2;
		uint8_t _res : 4;
		uint8_t caee : 1;
		uint8_t nop  : 1;
		UNION_STRUCT_END;
	} ceei;

	struct {
		UNION_STRUCT_START(8);
		uint8_t seei : 2;
		uint8_t _res : 4;
		uint8_t saee : 1;
		uint8_t nop  : 1;
		UNION_STRUCT_END;
	} seei;

	struct {
		UNION_STRUCT_START(8);
		uint8_t cerq : 2;
		uint8_t _res : 4;
		uint8_t caer : 1;
		uint8_t nop  : 1;
		UNION_STRUCT_END;
	} cerq;

	struct {
		UNION_STRUCT_START(8);
		uint8_t serq : 2;
		uint8_t _res : 4;
		uint8_t saer : 1;
		uint8_t nop  : 1;
		UNION_STRUCT_END;
	} serq;

	struct {
		UNION_STRUCT_START(8);
		uint8_t cdne : 2;
		uint8_t _res : 4;
		uint8_t cadn : 1;
		uint8_t nop  : 1;
		UNION_STRUCT_END;
	} cdne;

	struct {
		UNION_STRUCT_START(8);
		uint8_t ssrt : 2;
		uint8_t _res : 4;
		uint8_t sast : 1;
		uint8_t nop  : 1;
		UNION_STRUCT_END;
	} ssrt;

	struct {
		UNION_STRUCT_START(8);
		uint8_t cerr : 2;
		uint8_t _res : 4;
		uint8_t caei : 1;
		uint8_t nop  : 1;
		UNION_STRUCT_END;
	} cerr;

	struct {
		UNION_STRUCT_START(8);
		uint8_t cint : 2;
		uint8_t _res : 4;
		uint8_t cair : 1;
		uint8_t nop  : 1;
		UNION_STRUCT_END;
	} cint;

	uint32_t _pad2;

	struct {
		UNION_STRUCT_START(32);
		uint32_t int0 : 1;
		uint32_t int1 : 1;
		uint32_t int2 : 1;
		uint32_t int3 : 1;
		uint32_t _res : 28;
		UNION_STRUCT_END;
	} _int;

	uint32_t _pad3;

	struct {
		UNION_STRUCT_START(32);
		uint32_t err0 : 1;
		uint32_t err1 : 1;
		uint32_t err2 : 1;
		uint32_t err3 : 1;
		uint32_t _res : 28;
		UNION_STRUCT_END;
	} err;

	uint32_t _pad4;

	struct {
		UNION_STRUCT_START(32);
		uint32_t hrs0 : 1;
		uint32_t hrs1 : 1;
		uint32_t hrs2 : 1;
		uint32_t hrs3 : 1;
		uint32_t _res : 28;
		UNION_STRUCT_END;
	} hrs; // 4000_8034h

	uint8_t _pad5[0x100 - 0x38];

	struct {
		UNION_STRUCT_START(8);
		uint8_t chpri : 2;
		uint8_t _res  : 4;
		uint8_t dpa   : 1;
		uint8_t ecp   : 1;
		UNION_STRUCT_END;
	} dchpri[4]; // 4000_8100h .. 4000_8103h

	uint8_t _pad6[0x1000 - 0x104];

	struct DMA_TCD_t {
		uint32_t saddr; // 4000_9000h .. 4000_9060h

		uint16_t soff; // 4000_9004h .. 4000_9064h

		struct {
			UNION_STRUCT_START(16);
			enum dma_transfer_size_t dsize : 3;
			uint16_t dmod                  : 5;
			enum dma_transfer_size_t ssize : 3;
			uint16_t smod                  : 5;
			UNION_STRUCT_END;
		} attr; // 4000_9006h .. 4000_9066h

		union {
			struct {
				uint32_t nbytes;
			} mlno;
			struct {
				uint32_t nbytes : 30;
				uint32_t dmloe  : 1;
				uint32_t smloe  : 1;
			} mloffno;
			struct {
				uint32_t nbytes : 9;
				uint32_t mloff  : 21;
				uint32_t dmloe  : 1;
				uint32_t smloe  : 1;
			} mloffyes;
		} nbytes; // 4000_9008h .. 4000_9068h

		uint32_t slast; // 4000_900Ch .. 4000_906Ch

		uint32_t daddr; // 4000_9010h .. 4000_9070h

		uint16_t doff; // 4000_9014h .. 4000_9074h

		union {
			struct {
				uint16_t citer  : 9;
				uint16_t linkch : 2;
				uint16_t _res   : 4;
				uint16_t elink  : 1;
			} elinkyes;
			struct {
				uint16_t citer  : 15;
				uint16_t elink  : 1;
			} elinkno;
		} citer; // 4000_9016h .. 4000_9076h

		uint32_t dlastsga; // 4000_9018h .. 4000_9078h

		struct {
			UNION_STRUCT_START(16);
			uint16_t start       : 1;
			uint16_t intmajor    : 1;
			uint16_t inthalf     : 1;
			uint16_t dreq        : 1;
			uint16_t esg         : 1;
			uint16_t majorrelink : 1;
			uint16_t active      : 1;
			uint16_t done        : 1;
			uint16_t majorlinkch : 2;
			uint16_t _res        : 4;
			uint16_t bwc         : 2;
			UNION_STRUCT_END;
		} csr; // 4000_901Ch .. 4000_907Ch

		union {
			struct {
				UNION_STRUCT_START(16);
				uint16_t biter  : 9;
				uint16_t linkch : 2;
				uint16_t _res   : 4;
				uint16_t elink  : 1;
				UNION_STRUCT_END;
			} elinkyes;
			struct {
				UNION_STRUCT_START(16);
				uint16_t biter  : 15;
				uint16_t elink  : 1;
				UNION_STRUCT_END;
			} elinkno;
		} biter; // 4000_901Eh .. 4000_907Eh (TCD4_BITER_ELINKYES 4000_909Eh makes no sense)
	} tcd[4];
};

CTASSERT_SIZE_BYTE(struct DMA_t, 0x40009080 - 0x40008000);

enum dma_mux_source_t {
	DMA_MUX_SRC_UART0_RECV = 0x2,
	DMA_MUX_SRC_UART0_TRNS = 0x3,
	DMA_MUX_SRC_UART1_RECV = 0x4,
	DMA_MUX_SRC_UART1_TRNS = 0x5,
	DMA_MUX_SRC_UART2_RECV = 0x6,
	DMA_MUX_SRC_UART2_TRNS = 0x7,
	DMA_MUX_SRC_I2S_RECV   = 0xe,
	DMA_MUX_SRC_I2S_TRNS   = 0xf,
	DMA_MUX_SRC_SPI0_RECV  = 0x10,
	DMA_MUX_SRC_SPI0_TRNS  = 0x11,
	DMA_MUX_SRC_I2C0       = 0x16,
	DMA_MUX_SRC_FTM0_CH0   = 0x18,
	DMA_MUX_SRC_FTM0_CH1   = 0x19,
	DMA_MUX_SRC_FTM0_CH2   = 0x1a,
	DMA_MUX_SRC_FTM0_CH3   = 0x1b,
	DMA_MUX_SRC_FTM0_CH4   = 0x1c,
	DMA_MUX_SRC_FTM0_CH5   = 0x1d,
	DMA_MUX_SRC_FTM0_CH6   = 0x1e,
	DMA_MUX_SRC_FTM0_CH7   = 0x1f,
	DMA_MUX_SRC_FTM1_CH0   = 0x20,
	DMA_MUX_SRC_FTM2_CH1   = 0x21,
	DMA_MUX_SRC_ADC0       = 0x28,
	DMA_MUX_SRC_CMP0       = 0x2a,
	DMA_MUX_SRC_CMP1       = 0x2b,
	DMA_MUX_SRC_CMT        = 0x2f,
	DMA_MUX_SRC_PDB        = 0x30,
	DMA_MUX_SRC_PORTA      = 0x31,
	DMA_MUX_SRC_PORTB      = 0x32,
	DMA_MUX_SRC_PORTC      = 0x33,
	DMA_MUX_SRC_PORTD      = 0x34,
	DMA_MUX_SRC_PORTE      = 0x35,
	DMA_MUX_SRC_ALWAYS0    = 0x36,
	DMA_MUX_SRC_ALWAYS1    = 0x37,
	DMA_MUX_SRC_ALWAYS2    = 0x38,
	DMA_MUX_SRC_ALWAYS3    = 0x39,
	DMA_MUX_SRC_ALWAYS4    = 0x3a,
	DMA_MUX_SRC_ALWAYS5    = 0x3b,
	DMA_MUX_SRC_ALWAYS6    = 0x3c,
	DMA_MUX_SRC_ALWAYS7    = 0x3d,
	DMA_MUX_SRC_ALWAYS8    = 0x3e,
	DMA_MUX_SRC_ALWAYS9    = 0x3f
};

struct DMAMUX_t {
	UNION_STRUCT_START(8);
	enum dma_mux_source_t source : 6;
	uint8_t trig                 : 1;
	uint8_t enbl                 : 1;
	UNION_STRUCT_END;
};

CTASSERT_SIZE_BYTE(struct DMAMUX_t, 1);

extern volatile struct DMA_t DMA;
extern volatile struct DMAMUX_t DMAMUX0[16];
