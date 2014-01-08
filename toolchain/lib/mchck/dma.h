enum dma_arbitration_t {
	DMA_ARBITRATION_FIXED_PRIORITY = 0x0,
	DMA_ARBITRATION_ROUND_ROBIN    = 0x1
};

enum dma_channel {
	DMA_CH_0 = 0x0,
	DMA_CH_1 = 0x1,
	DMA_CH_2 = 0x2,
	DMA_CH_3 = 0x3
};

#define DMA_ERR_DST_BUS      0x1
#define DMA_ERR_SRC_BUS      0x2
#define DMA_ERR_SG_CONF      0x4
#define DMA_ERR_NBYTES_CITER 0x8
#define DMA_ERR_DST_OFF      0x10
#define DMA_ERR_DST_ADDR     0x20
#define DMA_ERR_SRC_OFF      0x40
#define DMA_ERR_SRC_ADDR     0x80
#define DMA_ERR_CHNL_PRIO    0x100
#define DMA_ERR_TRNFS_CANCL  0x200

typedef void (dma_cb)(enum dma_channel ch, uint32_t err, uint8_t major);

void dma_init(void);
void dma_set_arbitration(enum dma_arbitration_t arb);
void dma_from(enum dma_channel ch, void* addr, size_t count, enum dma_transfer_size_t tsize, size_t off, uint8_t mod);
void dma_to(enum dma_channel ch, void* addr, size_t count, enum dma_transfer_size_t tsize, size_t off, uint8_t mod);
void dma_major_loop_count(enum dma_channel ch, uint16_t iter);
void dma_start(enum dma_channel ch, enum dma_mux_source_t source, uint8_t trig, dma_cb* cb);
void dma_cancel(enum dma_channel ch);
void dma_set_priority(enum dma_channel ch, uint8_t prio);
void dma_enable_channel_preemption(enum dma_channel ch, uint8_t on);
void dma_enable_preempt_ability(enum dma_channel ch, uint8_t on);
void dma_from_addr_adj(enum dma_channel ch, uint32_t off);
void dma_to_addr_adj(enum dma_channel ch, uint32_t off);
