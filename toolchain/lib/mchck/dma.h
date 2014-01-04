enum dma_arbitration_t {
	DMA_ARBITRATION_FIXED_PRIORITY = 0x0,
	DMA_ARBITRATION_ROUND_ROBIN    = 0x1
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

typedef void (dma_callback)(uint8_t ch, uint32_t err);

void dma_init(void);
void dma_set_arbitration(enum dma_arbitration_t arb);
void dma_from(uint8_t ch, void* addr, size_t count, enum dma_transfer_size_t tsize, size_t off, uint8_t mod);
void dma_to(uint8_t ch, void* addr, size_t count, enum dma_transfer_size_t tsize, size_t off, uint8_t mod);
void dma_minor_loop(uint8_t ch, uint16_t iter);
void dma_major_loop(uint8_t ch, uint16_t iter);
void dma_start(uint8_t ch, dma_callback* cb);
void dma_set_priority(uint8_t ch, uint8_t prio);
void dma_enable_channel_preemption(uint8_t ch, uint8_t on);
void dma_enable_preempt_ability(uint8_t ch, uint8_t on);
