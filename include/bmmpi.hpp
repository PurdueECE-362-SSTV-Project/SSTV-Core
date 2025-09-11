#ifndef BMMPI_HPP
#define BMMPI_HPP
// BMMPI stands for Bare Metal MPI


#include <limits>
#include "pico/stdlib.h"
#include "pico/sync.h"
#include "types.hpp"


#define FIFO_TAIL INT32_MAX

typedef struct SFIFOMessage {
    int dest_id;
    int source_id;
    uint32_t* data;
    int data_size;
    int tail = FIFO_TAIL;
} SFIFOMessage;
SFIFOMessage construct_message(int dest_id, int source_id, uint32_t* data, int data_size);


typedef struct RFIFOMessage {
    uint32_t* data;
    int data_size;
} RFIFOMessage;


template <typename T>
class RingQueue {
    private: 
        T* data;
        unsigned int total_size;
        unsigned int head = 0;
        unsigned int tail = 0;
        int count = 0;

    public:
        RingQueue(int size);

        int size();
        int max_size();
        bool empty();
        bool full();

        static int wraparound_increment(int current, int max);

        Optional<T> pop_front();
        bool push_back(const T value);

        ~RingQueue();
};


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
        MessageLoader loader;
        RingQueue<RFIFOMessage>* internal_queues; // index corresponds to target id
        int num_queues = 0;
    protected:
        bool atomic_push_proc(RFIFOMessage value) override;
        Optional<RFIFOMessage> atomic_pop_proc() override;

        ~ReceiverQueue();
};


class ReceiverQueueBuilder {
    private: 
        RingQueue<RFIFOMessage>* internal_queues; // index corresponds to target id how does receiver know which queue to eat from?
        int num_queues = 0;
        int max_queues;
    public:
        ReceiverQueueBuilder(int max_queues);
        ReceiverQueueBuilder attach_queue(int size);
        ReceiverQueue finish_building(); // make sure to actually hand off the pointer to the queue so the builder deallocating doesnt leave dangling pointer

        ~ReceiverQueueBuilder();
};


typedef struct MPISR {
    void* isr;
    int priority;
} MPISR;


class MPSender {
    private:
    
};

// add memory sharing here


#endif