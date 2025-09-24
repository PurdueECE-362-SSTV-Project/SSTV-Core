#ifndef DMA_CONFIG_H
#define DMA_CONFIG_H


#include <cstdint>
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


typedef enum {
    DREQ_PIO0_TX0     = 0,
    DREQ_PIO0_TX1     = 1,
    DREQ_PIO0_TX2     = 2,
    DREQ_PIO0_TX3     = 3,
    DREQ_PIO0_RX0     = 4,
    DREQ_PIO0_RX1     = 5,
    DREQ_PIO0_RX2     = 6,
    DREQ_PIO0_RX3     = 7,
    DREQ_PIO1_TX0     = 8,
    DREQ_PIO1_TX1     = 9,
    DREQ_PIO1_TX2     = 10,
    DREQ_PIO1_TX3     = 11,
    DREQ_PIO1_RX0     = 12,
    DREQ_PIO1_RX1     = 13,
    DREQ_PIO1_RX2     = 14,
    DREQ_PIO1_RX3     = 15,
    DREQ_PIO2_TX0     = 16,
    DREQ_PIO2_TX1     = 17,
    DREQ_PIO2_TX2     = 18,
    DREQ_PIO2_TX3     = 19,
    DREQ_PIO2_RX0     = 20,
    DREQ_PIO2_RX1     = 21,
    DREQ_PIO2_RX2     = 22,
    DREQ_PIO2_RX3     = 23,
    DREQ_PIO3_TX0     = 24,
    DREQ_PIO3_TX1     = 25,
    DREQ_PIO3_TX2     = 26,
    DREQ_PIO3_TX3     = 27,
    DREQ_PIO3_RX0     = 28,
    DREQ_PIO3_RX1     = 29,
    DREQ_PIO3_RX2     = 30,
    DREQ_PIO3_RX3     = 31,
    DREQ_SPI0_TX      = 32,
    DREQ_SPI0_RX      = 33,
    DREQ_SPI1_TX      = 34,
    DREQ_SPI1_RX      = 35,
    DREQ_UART0_TX     = 36,
    DREQ_UART0_RX     = 37,
    DREQ_UART1_TX     = 38,
    DREQ_UART1_RX     = 39,
    DREQ_PWM_WRAP0    = 40,
    DREQ_PWM_WRAP1    = 41,
    DREQ_PWM_WRAP2    = 42,
    DREQ_PWM_WRAP3    = 43,
    DREQ_PWM_WRAP4    = 44,
    DREQ_PWM_WRAP5    = 45,
    DREQ_PWM_WRAP6    = 46,
    DREQ_PWM_WRAP7    = 47,
    DREQ_I2C0_TX      = 48,
    DREQ_I2C0_RX      = 49,
    DREQ_I2C1_TX      = 50,
    DREQ_I2C1_RX      = 51,
    DREQ_ADC          = 52,
    DREQ_XIP_STREAM   = 53,
    DREQ_XIP_SSITX    = 54,
    DREQ_XIP_SSIRX    = 55,
    DREQ_XIP_QMITX    = 56,
    DREQ_XIP_QMIRX    = 57,
    DREQ_JSTX         = 58,
    DREQ_JSRX         = 59,
    DREQ_CORESIGHT    = 60,
    DREQ_SHA256       = 61
} DREQNum;


void init_dma(uint32_t* read_addr, uint32_t* write_addr, DREQNum dreq_num, uint32_t transfer_count, DMAMode mode);

void init_dma_irq(uint32_t channel_num, void* irq_handler);

#endif // DMA_CONFIG_H