#include "llmp16.h"


void llmp16_dma_init(llmp16_dma_t *dma)
{
    dma->count = 0;
    dma->ctrl = 0;
    dma->dst_addr = 0;
    dma->src_addr = 0;
    dma->stat = 0;
    dma->irq_line = 0;
}

void llmp16_dma_cpy(llmp16_t *vm, llmp16_dma_t *dma)
{
    uint8_t value;
    if (dma->count > 0x8000)
    {
        dma->stat |= DMA_STAT_ERROR;
        return;
    }
    
    for (uint16_t offset = 0; offset < dma->count; offset++)
    {
        value = mem_read8(vm, dma->src_addr+offset);
        vram_write(vm, dma->dst_addr+offset, value);
    }
}

void llmp16_dma_readIO(llmp16_t *vm, llmp16_dma_t *dma)
{
    dma->count = vm->IO[LLMP_DMA_PORT][LLMP_DMA_REG_CNT];
    dma->ctrl = vm->IO[LLMP_DMA_PORT][LLMP_DMA_REG_CTRL];
    dma->dst_addr = vm->IO[LLMP_DMA_PORT][LLMP_DMA_REG_DST];
    dma->src_addr = vm->IO[LLMP_DMA_PORT][LLMP_DMA_REG_SRC];
    dma->stat = vm->IO[LLMP_DMA_PORT][LLMP_DMA_REG_STAT];
    dma->irq_line = 0;
}


void llmp16_dma_step(llmp16_t *vm, llmp16_dma_t *dma)
{
    llmp16_dma_readIO(vm, dma);
    if ((dma->ctrl & DMA_CTRL_ENABLE)) {

        llmp16_dma_cpy(vm, dma);

        dma->stat |= DMA_STAT_DONE;

        if (dma->ctrl & DMA_CTRL_IRQ_EN) {
            dma->irq_line = true;
            // TODO : interruputions
        }
    }
}