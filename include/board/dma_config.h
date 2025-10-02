#ifndef DMA_CONFIG_H
#define DMA_CONFIG_H


#include <stdint.h>
#include "pico/stdlib.h"
#include "pico/sync.h"
#include "pico/multicore.h"
#include "hardware/dma.h"


#define DMA_IRQ_OFFSET_VAL DMA_IRQ_0;

typedef enum DMAMode {
    DMA_MODE_NORMAL,
    DMA_MODE_TRIGGER_SELF,
    DMA_MODE_ENDLESS
} DMAMode;


void init_dma(uint32_t* read_addr, uint32_t* write_addr, int dreq_num, uint32_t transfer_count, DMAMode mode);

void init_dma_irq(uint32_t channel_num, void* irq_handler);

#endif // DMA_CONFIG_H