#ifndef BMMPI_HPP
#define BMMPI_HPP
// BMMPI stands for Bare Metal MPI


#include <limits>
#include "pico/stdlib.h"
#include "pico/sync.h"
#include "pico/multicore.h"
#include "hardware/dma.h"

#include "native/n_bmmpi.hpp"
#include "native/n_types.hpp"
#include "AppConfig.h"


#if ENABLE_FIFO_MESSAGING == 1

extern ReceiverRouter fifo_queue_core0; // input to core 0
extern ReceiverRouter fifo_queue_core1; // input to core 1

void fifo_core0_irq();
void fifo_core1_irq();

void init_fifo_irq_core0();
void init_fifo_irq_core1();


template <typename T>
class AtomicQueue {
    protected: 
        critical_section at_queue_cs;
        virtual bool atomic_push_proc(T value);
        virtual T atomic_pop_proc();
    public:
        bool is_full();
        bool is_empty();
        bool atomic_push(T value);
        T atomic_pop();
};


class SenderQueue : protected AtomicQueue<FIFOMessage> {
    private:
        RingQueue<FIFOMessage> internal_queue;
    protected:
        bool atomic_push_proc(FIFOMessage value) override;
        FIFOMessage atomic_pop_proc() override;
};


#include "board/templates/atomic_queue.tpp"


#endif // ENABLE_FIFO_MESSAGING == 1

#if ENABLE_SHARED_MEMORY == 1

template <typename T, unsigned int SIZE>
using BufferAccessFunction = void (*)(T buffer[SIZE]);

template <typename T, unsigned int SIZE>
class SharableBuffer {
    private:
        T buffer[SIZE];
        mutex_t buffer_mutex;
    public:
        static_assert(SIZE & (SIZE - 1) == 0, "SIZE must be a power of 2");

        SharableBuffer();
        unsigned int size();
        T* acquire(bool blocking = true);
        void release();
        bool acquire_and_run(BufferAccessFunction<T, SIZE> func, bool blocking = true);
};

#endif // ENABLE_SHARED_MEMORY == 1

#if ENABLE_DMA_MESSAGING == 1

#if MESSAGE_BUFFER_SIZE & (MESSAGE_BUFFER_SIZE - 1) != 0
    #error "MESSAGE_BUFFER_SIZE must be a power of 2"
#endif

#ifdef MAX_MESSAGE_QUEUES
    #if MAX_MESSAGE_QUEUES < 1 || MAX_MESSAGE_QUEUES > 6
        #error "MAX_MESSAGE_QUEUES must be between 1 and 5"
    #endif
    #define AVAIL_RECV_IRQS {SPAREIRQ_IRQ_0, SPAREIRQ_IRQ_1, SPAREIRQ_IRQ_2, SPAREIRQ_IRQ_3, SPAREIRQ_IRQ_4, SPAREIRQ_IRQ_5}
#else
    #error "MAX_MESSAGE_QUEUES must be defined in AppConfig.h"
#endif


extern uint8_t message_channel_count;
int claim_message_channel_number();


typedef enum MessageStatus {
    MESSAGE_IDLE,
    MESSAGE_AWAITING_READY,
    MESSAGE_AWAITING_COMPLETE // there is no send awaiting in the queue
} MessageStatus;


typedef struct MessageChannel { // the receiver must be able to set the MESSAGE enable of the sender
    int dma_channel_number;
    int interrupt_channel_number;
    dma_channel_config channel_config;
    volatile MessageStatus status = MESSAGE_IDLE;
    uint32_t* queued_sending_address;
    uint32_t* receiving_buffer;
} MessageChannel;


class MessageReceiver {
    private:
        volatile MessageChannel& channel;
        int interrupt_channel_number;
    public:
        MessageReceiver(MessageChannel& channel);
        bool receive();
};


class MessageSender {
    private:
        volatile MessageChannel& channel;
    public:
        MessageSender(MessageChannel& channel);
        bool send(uint32_t source_address[DMA_BUFFER_SIZE]);
};


bool channel(uint32_t dest_address[DMA_BUFFER_SIZE], MessageSender* sender_out, MessageReceiver* receiver_out, volatile MessageChannel* channel);


#endif // ENABLE_DMA_MESSAGING == 1


#endif