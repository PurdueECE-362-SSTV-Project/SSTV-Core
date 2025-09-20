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


// these abstract methods are provided to future proof behavior of specific atomic queue implementations. Not doing much right now tho
bool SenderQueue::atomic_push_proc(FIFOMessage value) {
    return this->internal_queue.push_back(value);
}


FIFOMessage SenderQueue::atomic_pop_proc() {
    return this->internal_queue.pop_front();
}

// *************************************************
// MESSAGING USING DMA
// *************************************************


uint8_t message_channel_count = 0;
int claim_message_channel_number() {
    if(message_channel_count >= MAX_MESSAGE_QUEUES) {
        return -1; // no more channels available
    }
    message_channel_count++;
    return AVAIL_RECV_IRQS[message_channel_count - 1];
}


MessageSender::MessageSender(MessageChannel& channel) : channel(channel) {}


bool MessageSender::send(uint32_t source_address[DMA_BUFFER_SIZE]) {
    // if not already pending on a send, activate ISR to commence send when the ready_flag is set
    if(this->channel.status == MESSAGE_AWAITING_READY) {
        return false; // there is already another queued send registered to interrupt
    }
    if (this->channel.status == MESSAGE_IDLE) {
        dma_channel_configure(
            this->channel.channel_number, 
            &this->channel.channel_config,
            this->channel.receiving_buffer, 
            source_address,
            DMA_BUFFER_SIZE, // transfer count
            true
        );
        this->channel.status = MESSAGE_AWAITING_COMPLETE;

        return true;
    }
    else if(this->channel.status == MESSAGE_AWAITING_COMPLETE) { // must await interrupt call here
        this->channel.queued_sending_address = source_address;
        this->channel.status = MESSAGE_AWAITING_READY;
        return true;
    }
}


bool channel(uint32_t dest_address[DMA_BUFFER_SIZE], MessageSender* sender_out, MessageReceiver* receiver_out, volatile MessageChannel* chan) { // messag echannel should eb stored as volatile
    int dma_channel_number = dma_claim_unused_channel(true);
    int interrupt_channel_number = claim_message_channel_number();

    if(interrupt_channel_number == -1) {
        return false;
    }

    chan->dma_channel_number = dma_channel_number;
    chan->interrupt_channel_number = interrupt_channel_number;
    chan->channel_config = dma_channel_get_default_config(channel_number);
    chan->queued_sending_address = source_address;
    chan->receiving_buffer = dest_address;

    channel_config_set_transfer_data_size(&chan->channel_config, DMA_SIZE_32);
    channel_config_set_read_increment(&chan->channel_config, true);
    channel_config_set_write_increment(&chan->channel_config, true);

    dma_channel_configure(
        channel_number, 
        &chan->channel_config,
        dest_address,
        nullptr,
        DMA_BUFFER_SIZE, // transfer count
        false // don't start yet
    );

    *sender_out = MessageSender(chan);
    *receiver_out = MessageReceiver(chan)

    return true;
}