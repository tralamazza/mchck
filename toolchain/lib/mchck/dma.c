#include <mchck.h>

static struct dma_ctx_t {
	dma_callback* cb;
} ctx[4];

void
dma_init(void)
{
	/* set some defaults */
	int i;
	for (i = 0; i < 4; i++) {
		volatile struct DMA_TCD_t* tcd = &DMA.tcd[i];
		tcd->soff = 0;
		tcd->attr.raw = 0;
		tcd->slast = 0;
		tcd->doff = 0;
		tcd->citer.elinkno.citer = 1;
		tcd->citer.elinkno.elink = 0;
		tcd->dlastsga = 0;
		tcd->csr.intmajor = 1; /* enable interrupt after biter finishes */
		tcd->csr.majorlinkch = 0; /* don't chain (link) after biter */
		tcd->csr.bwc = 0; /* no dma stalls */
		tcd->biter.elinkno.biter = 1;
		tcd->biter.elinkno.elink = 0;
	}
	DMA.seei.saee = 1; /* enable all error interrupts */
	int_enable(IRQ_DMA0);
	int_enable(IRQ_DMA1);
	int_enable(IRQ_DMA2);
	int_enable(IRQ_DMA3);
	int_enable(IRQ_DMA_error);
}

void
dma_set_arbitration(enum dma_arbitration_t arb)
{
	DMA.cr.erca = arb;
}

void
dma_from(uint8_t ch, void* addr, size_t count, enum dma_transfer_size_t tsize, size_t off, uint8_t mod)
{
	volatile struct DMA_TCD_t* tcd = &DMA.tcd[ch];
	tcd->saddr = (uint32_t)addr;
	tcd->soff = off;
	tcd->attr.ssize = tsize;
	tcd->attr.smod = mod;
}

void
dma_to(uint8_t ch, void* addr, size_t count, enum dma_transfer_size_t tsize, size_t off, uint8_t mod)
{
	volatile struct DMA_TCD_t* tcd = &DMA.tcd[ch];
	tcd->daddr = (uint32_t)addr;
	tcd->doff = off;
	tcd->attr.dsize = tsize;
	tcd->attr.dmod = mod;
}

void
dma_minor_loop(uint8_t ch, uint16_t iter)
{
	DMA.tcd[ch].citer.elinkno.elink = 0;
	DMA.tcd[ch].citer.elinkno.citer = iter;
}

void
dma_major_loop(uint8_t ch, uint16_t iter)
{
	DMA.tcd[ch].biter.elinkno.elink = 0;
	DMA.tcd[ch].biter.elinkno.biter = iter;
}

void
dma_start(uint8_t ch, enum dma_mux_source_t source, uint8_t tri, dma_callback* cb)
{
	ctx[ch].cb = cb;
	volatile struct DMAMUX_t *mux = &DMAMUX0[ch];
	mux->trig = 0;
	mux->enbl = 0;
	mux->source = source;
	DMA.tcd[ch].csr.start = 1;
	if (mux->trig)
		mux->trig = 1;
	mux->enbl = 1;
}

void
dma_set_priority(uint8_t ch, uint8_t prio)
{
	DMA.dchpri[ch].chpri = prio;
}

void
dma_enable_channel_preemption(uint8_t ch, uint8_t on)
{
	DMA.dchpri[ch].ecp = on;
}

void
dma_enable_preempt_ability(uint8_t ch, uint8_t on)
{
	DMA.dchpri[ch].dpa = !on;
}

void
DMA0_Handler(void)
{

}

void
DMA1_Handler(void)
{

}

void
DMA2_Handler(void)
{

}

void
DMA3_Handler(void)
{

}

void
DMA_error_Handler(void)
{
	uint32_t err = *(uint8_t*)&DMA.es;
	if (DMA.es.cpe)
		err |= DMA_ERR_CHNL_PRIO;
	if (DMA.es.ecx)
		err |= DMA_ERR_TRNFS_CANCL;
	ctx[DMA.es.errchn].cb(DMA.es.errchn, err, 0);
	DMA.cerr.cerr = DMA.es.errchn;
}

