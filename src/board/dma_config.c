#include "dma_config.h"


static uint32_t dma_irq_num = 0;


void init_dma(uint32_t* read_addr, uint32_t* write_addr, DREQNum dreq_num, uint32_t transfer_count, DMAMode mode) {
    if(transfer_count >= (1 << 28)) {
        panic("DMA transfer count too large");
    }
    uint32_t channel = dma_claim_unused_channel(true);

    dma_channel_hw_addr(channel)->read_addr = (uintptr_t) read_addr;
    dma_channel_hw_addr(channel)->write_addr = (uintptr_t) write_addr;

    dma_channel_hw_addr(channel)->transfer_count = (uint32_t) transfer_count | (mode << 28);

    dma_hw->ch[channel].ctrl_trig = 0;
    uint32_t temp = 0;
    temp |= (DMA_SIZE_32 << 2) | (dreq_num << 17) | 1;

    dma_hw->ch[channe].ctrl_trig = temp;
}


void enable_dma_irq(uint32_t channel_num, void* irq_handler, bool enable) {
    switch (dma_irq_num) {
        io_rw_32* inte_val
        case 0:
            inte_val = &dma_hw->inte0;
            dma_irq_num++;
            break;
        case 1:
            inte_val = &dma_hw->inte1;
            dma_irq_num++;
            break;
        case 2:
            inte_val = &dma_hw->inte2;
            dma_irq_num++;
            break;
        case 3:
            inte_val = &dma_hw->inte0;
            dma_irq_num++;
            break;
        default:
            panic("DMA IRQ number too large");
    }

    if(enable) {
        hw_set_bits(&dma_hw->inte0, 1 << channel_num);
    } 
    else {
        hw_clear_bits(&dma_hw->inte0, 1 << channel_num);
    }
}