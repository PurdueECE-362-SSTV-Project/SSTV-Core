#ifndef BMMPI_NAT_HPP
#define BMMPI_NAT_HPP


#include <limits>
#include "native/types.hpp"

#include "AppConfig.h"

#if ENABLE_FIFO_MESSAGING == 1
#define FIFO_TAIL INT32_MAX
#endif

typedef struct SFIFOMessage {
    int dest_id;
    int source_id;
    int data_size;
    uint32_t* data;
    int tail = FIFO_TAIL;
} SFIFOMessage;
SFIFOMessage construct_message(int dest_id, int source_id, uint32_t* data, int data_size);


typedef struct RFIFOMessage {
    uint32_t* data;
    int data_size;
} RFIFOMessage;


#ifndef RING_QUEUE_BUFFER_SIZE
#define RING_QUEUEBUFFER_SIZE 256
#endif

#if (RING_QUEUE_BUFFER_SIZE & (RING_QUEUE_BUFFER_SIZE - 1)) != 0
#error "BUFFER_SIZE must be a power of 2"
#endif

#define RING_QUEUE_BUFFER_MASK (RING_QUEUE_BUFFER_SIZE - 1)

template <typename T>
class RingQueue {
    private: 
        T data[RING_QUEUE_BUFFER_SIZE];
        T default_value;
        unsigned int head = 0;
        unsigned int tail = 0;
        int count = 0;
    public:
        RingQueue(T default_value);
        int size();
        int max_size();
        bool empty();
        bool full();

        static int wraparound_increment(int current);

        void flush();

        T pop_front();
        bool push_back(const T value);
};


#include "templates/ring_queue.tpp"

#endif