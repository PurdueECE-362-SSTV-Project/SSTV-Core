#include <stdio.h>
#include "config/AppConfig.h"
#include "board/bmmpi.hpp"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "hardware/xosc.h"



// *************************************************
// MESSAGING USING FIFO QUEUE
// *************************************************
// MUST LAUNCH THE OTHER CORE BEFORE DOING ANY FIFO SETUP
void init_fifo_irq_core0() {
    irq_set_exclusive_handler(SIO_FIFO_IRQ_NUM(0), fifo_core0_irq);
    irq_set_enabled(SIO_FIFO_IRQ_NUM(0), true);
    #ifdef FIFO_IRQ_PRIORITY
    irq_set_priority(SIO_FIFO_IRQ_NUM(0), FIFO_IRQ_PRIORITY);
    #endif
}


void init_fifo_irq_core1() {
    irq_set_exclusive_handler(SIO_FIFO_IRQ_NUM(1), fifo_core1_irq);
    irq_set_enabled(SIO_FIFO_IRQ_NUM(1), true);
    #ifdef FIFO_IRQ_PRIORITY
    irq_set_priority(SIO_FIFO_IRQ_NUM(1), FIFO_IRQ_PRIORITY);
    #endif
}


void fifo_core0_irq() {
    while (multicore_fifo_rvalid()) {
        FIFOMessage msg = message_from_uint32(multicore_fifo_pop_blocking());
        fifo_queue_core0.route_message(msg);
    }

    multicore_fifo_clear_irq();
}


void fifo_core1_irq() {
    while (multicore_fifo_rvalid()) {
        FIFOMessage msg = message_from_uint32(multicore_fifo_pop_blocking());
        fifo_queue_core1.route_message(msg);
    }

    multicore_fifo_clear_irq();
}


// *************************************************
// MESSAGING USING DMA
// *************************************************


// uint8_t message_channel_count = 0;
// int claim_message_channel_number() {
//     if(message_channel_count >= MAX_MESSAGE_QUEUES) {
//         return -1; // no more channels available
//     }
//     message_channel_count++;
//     return AVAIL_RECV_IRQS[message_channel_count - 1];
// }


// MessageSender::MessageSender(MessageChannel& channel) : channel(channel) {}


// *************************************************
// MESSAGING USING THREAD SAFE QUEUES
// *************************************************

template <typename T, size_t N, size_t W>
MerryMemoryOrigin<T, N, W>::MerryMemoryOrigin() {
    for(int buffer_number = 0; buffer_number < N; buffer_number++) {
        unique_ptr<MerryBuffer<T, W>> buffer = std::make_unique<MerryBuffer<T, W>>();
        this->push_back(buffer);
    }
}