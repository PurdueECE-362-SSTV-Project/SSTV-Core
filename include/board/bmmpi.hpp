#ifndef BMMPI_HPP
#define BMMPI_HPP
// BMMPI stands for Bare Metal MPI


#include <limits>
#include <cstdint>
#include <memory>
#include <atomic>
#include <vector>
#include "pico/stdlib.h"
#include "pico/sync.h"
#include "pico/multicore.h"
#include "hardware/dma.h"

#include "native/n_bmmpi.hpp"
#include "native/n_types.hpp"
#include "AppConfig.h"


using namespace std;


template <typename T, int N>
class InterruptQueue : protected RingQueue<T, N> {
    protected: 
        critical_section at_queue_cs;
    public:
        void flush() override;
        T pop_front(bool* result) override;
        bool push_back(const T value) override;
};


template <typename T, size_t N>
class MulticoreLocklessQueueSPSC : protected BaseQueue<T, N> {
    protected: 
        atomic<size_t> head = 0;
        atomic<size_t> tail = 0;
    public:
        void flush() override;
        T pop_front(bool* result) override;
        bool push_back(const T value) override;
};


#if ENABLE_FIFO_MESSAGING == 1

extern ReceiverRouter fifo_queue_core0; // input to core 0
extern ReceiverRouter fifo_queue_core1; // input to core 1

void fifo_core0_irq();
void fifo_core1_irq();

void init_fifo_irq_core0();
void init_fifo_irq_core1();


#include "board/templates/atomic_queue.tpp"


#endif // ENABLE_FIFO_MESSAGING == 1

#if ENABLE_MERRY_MEMORY == 1 


template<typename T, size_t W>
struct _MerryBuffer {
    uint32_t id;
    T buffer[W];
};


template <typename T, size_t W>
using MerryBuffer = _MerryBuffer<T, W>;


/**
 * @brief memory origin used to supply addresses to queue-based pipelines.
 *
 * This class is intended to be implemented as a singleton: every program
 * should have exactly one `MerryMemoryOrigin`. For queue-based memory
 * address pipelines, the origin is the single authoritative allocator and
 * deallocator. As the pipeline starts, the origin will allocate and
 * deallocate memory in one place and "pump" addresses into the pipeline's
 * inbound queue. Once the origin observes that all addresses are enqueued
 * in the pipeline, the pipeline stage connected to the origin will only
 * receive address pushes from the task assigned to its inbound queue.
 *
 * In short: this centralizes address management for pipeline-based memory
 * flows and guarantees a single source of allocation for the pipeline.
 */
template <typename T, size_t N, size_t W>
class MerryMemoryOrigin : protected MulticoreLocklessQueueSPSC<MerryBuffer<T, W>, N> {
    public: 
        MerryMemoryOrigin();
};

template <typename T, int W>
class MerryTaskFunction {
    public:
    virtual bool run(unique_ptr<MerryBuffer<T, W>>& buffer_span);
};


typedef void(*TaskManagerFunction)();
typedef struct MerryTaskInterrupt {
    unsigned int irq_num = 0;
    TaskManagerFunction acknowledge;
} MerryTaskInterrupt;


/**
 * @brief Aggregates task execution, interrupt management, and queue pipelining.
 *
 * For simplicity, `MerryTask` is a helper template that ties together a
 * task function, its inbound/outbound queue interfaces, and the interrupt
 * handling needed to drive a queue-based pipeline stage. IO bound tasks like ADC or DMA subroutines should be using register_interrupt
 * while compute heavy tasks like signal processing should just be done with call_synchronous in the main loop on one of the cores. Remember you can have only one per core
 */

#define NUM_IRQS 52

template<typename T, size_t N, size_t W>
class MerryTask {
    private: 
        BaseQueue<unique_ptr<MerryBuffer<T, W>>, N>& inbound_queue;
        BaseQueue<unique_ptr<MerryBuffer<T, W>>, N> outbound_queue;
        MerryTaskFunction<T, W> function;

        char core = 0;
        long int loss_counter = 0;
    public:
        TaskManagerFunction set_up;
        TaskManagerFunction tear_down;
        MerryTaskInterrupt interrupt;

        MerryTask(MerryTaskFunction<T, W> function, TaskManagerFunction set_up, TaskManagerFunction tear_down, char core);
        bool call();
};


template<typename T, size_t N, size_t W>
class MerryTaskBuilder {
    private:
        unique_ptr<MerryTask<T, N, W>> irq_tasks[NUM_IRQS];
        MerryMemoryOrigin<T, N, W> origin_queue;
        BaseQueue<unique_ptr<MerryBuffer<T, W>>, N>& previous_task = this->origin_queue;
    public:
        bool register_irq_task(
            MerryTaskFunction<T, W> function, 
            TaskManagerFunction set_up, 
            TaskManagerFunction tear_down, 
            MerryTaskInterrupt interrupt,
            char core
        );
        unique_ptr<MerryTask<T, N, W>> register_main_loop_task(
            MerryTaskFunction<T, W> function, 
            TaskManagerFunction set_up, 
            TaskManagerFunction tear_down, 
            char core
        );

}


#define GENERATE_IRQ_FUNCTION(irqnum, acknowledge, type, size, width) \
type autogen_task_irq_##irqnum() { \
    MerryTask<type, size, width>::irq_tasks[irq_num].call_irq(); \
    acknowledge(); \
} \
irq_set_exclusive_handler(irq_num, autogen_task_irq_##irqnum); 


#include "board/templates/merry_go_round.tpp"

#endif // ENABLE_MERRYGOROUND_MEMORY == 1

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