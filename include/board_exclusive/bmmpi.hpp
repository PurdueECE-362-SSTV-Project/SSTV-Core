#ifndef BMMPI_HPP
#define BMMPI_HPP
// BMMPI stands for Bare Metal MPI


#include <limits>
#include "pico/stdlib.h"
#include "pico/sync.h"
#include "native/types.hpp"
#include "AppConfig.h"


class MessageUnloader {
    private:
        uint32_t* message = nullptr;
        int length = 0;
        int current_index = 0;
    public:
        bool assign_message(uint32_t* message, int length); // should return false if the whole message wasnt consumed
        void flush_message(uint32_t* message, int length); // force message overwrite
        bool is_empty(); // check if the current index equal to length
        uint32_t get_head();
        bool decrement_index(); // decrement the index for retries if the caller fails to do something
        ~MessageUnloader();
};


class MessageLoader {
    private:
        uint32_t* message = nullptr;
        int length = 0;
        int current_index = 0;
    public:
        bool start_message(int length); // return false if the existing message is not flushed (not nullptr)
        uint32_t* retrieve_message(); // retrieve the existing message
        bool append(uint32_t frame); // return true if the buffer is full after appending

        ~MessageLoader();
};


template <typename T>
class AtomicQueue {
    protected: 
        critical_section at_queue_cs;
        virtual bool atomic_push_proc(T value);
        virtual Optional<T> atomic_pop_proc();
    public:
        bool atomic_push(T value);
        Optional<T> atomic_pop();
};


class SenderQueue : protected AtomicQueue<SFIFOMessage> {
    private:
        MessageUnloader unloader;
        RingQueue<SFIFOMessage> internal_queue;
    protected:
        SenderQueue(int size);
        bool atomic_push_proc(SFIFOMessage value) override;
        Optional<SFIFOMessage> atomic_pop_proc() override;

        ~SenderQueue();
};


class ReceiverQueue: protected AtomicQueue<RFIFOMessage> {
    private:
        RingQueue<RFIFOMessage> internal_queue; // index corresponds to target id
    protected:
        bool atomic_push_proc(RFIFOMessage value) override;
        Optional<RFIFOMessage> atomic_pop_proc() override;

        ~ReceiverQueue();
};


// governs the state of the ReceiverQueueBlock for decoding messages and deciding where they go
typedef enum MessageReceiveStage {
    RECEIVE_DEST,
    RECEIVE_SOURCE,
    RECEIVE_SIZE,
    RECEIVE_DATA,
} MessageReceiveStage;


class ReceiverQueueBlock {
    private:
        int num_queues = 0;
        ReceiverQueue* internal_queues;
        MessageLoader loader;
        int selected_queue = 0;
    public:
        ReceiverQueueBlock(int num_queues, ReceiverQueue* queues);
        Optional<RFIFOMessage> pop(int queue_id);
        bool push(uint32_t frame); // must have a mechanism to notify the target task of a completed push to the queue so that can be popped and processed
        void trigger_task(int queue_id);

        ~ReceiverQueueBlock();
};


typedef struct MPISR {
    void* isr;
    int priority;
} MPISR;


class MPSender {
    private:
        SenderQueue queue;

    public:
        MPSender(MPISR submit_isr, MPISR receive_empty_isr, MPISR watchdog_isr, int max_queue_size);
};


class MPReceiver {
    private:
        ReceiverQueueBlock block;
    public:
        bool route_messages(); // return false if no messages were routed. This should retrieve from the FIFO queue
};


class MPReceiverBuilder {
    private: 
        ReceiverQueue* internal_queues; // index corresponds to target id how does receiver know which queue to eat from?
        int num_queues = 0;
        int max_queues;
    public:
        MPReceiverBuilder(int max_queues);
        MPReceiverBuilder add_receiver(void* MPISR, int buffer_size); // throw an error if ever exceed the max number of queues, since this happen only at startup
        // the ISR is to be triggered artificially by an unused GPIO interrupt register
        // this is done so that immediate execution isnt needed. If it is low priority, the interrupt trigger is just a statement of: "this will be done sometime in the future"
        MPReceiver finish_building(); // make sure to actually hand off the pointer to the queue so the builder deallocating doesnt leave dangling pointer

        ~MPReceiverBuilder();
};
#endif // ENABLE_FIFO_MESSAGING == 1

#if ENABLE_SHARED_MEMORY == 1

#endif // ENABLE_SHARED_MEMORY == 1


#endif