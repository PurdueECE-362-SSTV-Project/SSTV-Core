#ifndef APP_CONFIG_H
#define APP_CONFIG_H


#define ENABLE_MERRY_MEMORY 1
#define ENABLE_FIFO_MESSAGING 1
#define RING_QUEUE_BUFFER_SIZE 256 // number of uint32_t words in the buffer
#define FIFO_IRQ_PRIORITY 5 // IRQ priority for FIFO interrupts (0 is highest priority)
#define DMA_BUFFER_SIZE 1024 // number of uint32_t words in the DMA buffer
#define MERRY_BUFFER_SIZE 1024
#define MERRY_MEMORY_BLOCK_COUNT 256
#define MAX_MESSAGE_QUEUES 3


#endif