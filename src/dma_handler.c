#include <stdlib.h>
#include "hardware/dma.h"

#include "input_control.h"
#include "Ili9341.h"

void dma_irq_handler() {
    //printf("h\n");
    uint32_t status = dma_hw->ints0;

    // Clear interrupts for channels that triggered this IRQ
    dma_hw->ints0 = status;

    if (status & (1u << TFT_DMA_CH)) {
        dma_display_irq();
    }
    if (status & (1u << ADC_DMA_CH)) {
        dma_handler();
    }
}

void init_dma() {
    irq_set_exclusive_handler(DMA_IRQ_0, dma_irq_handler);
    irq_set_enabled(DMA_IRQ_0, true);
}
