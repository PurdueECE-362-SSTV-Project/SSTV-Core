#ifndef N_BMMPI_HPP
#define N_BMMPI_HPP


#include <limits.h>
#include <cstdint>
#include "native/n_types.hpp"

#include "AppConfig.h"

#if ENABLE_FIFO_MESSAGING == 1
#define FIFO_TAIL INT32_MAX
#endif


#define FIFO_DEST_ID_SIZE 3
#define FIFO_SOURCE_ID_SIZE 3
#define FIFO_DATA_SIZE 26

#if FIFO_DEST_ID_SIZE + FIFO_SOURCE_ID_SIZE + FIFO_DATA_SIZE != 32
#error "FIFO message bit sizes must add up to 32"
#endif

#define FIFO_DEST_ID_MAX ((1u << FIFO_DEST_ID_SIZE) - 1u)
#define FIFO_SOURCE_ID_MAX ((1u << FIFO_SOURCE_ID_SIZE) - 1u)
#define FIFO_DATA_MAX ((1u << FIFO_DATA_SIZE) - 1u)



/**
 * @brief Short FIFO message used for inter-core notifications.
 *
 * The FIFO queue should never be used for bulk data transfers. It is
 * solely for triggering interrupts between cores for IO-bound tasks,
 * i.e., as a notifier that something must be done. If an IO-bound
 * resource on one core (for example, an OLED) depends on an IO-bound
 * resource on another core (for example, an encoder), you should need
 * no more than the 26 provided data bits to indicate the required
 * action. For example, the 26-bit field can be used to index an image
 * stored elsewhere which should be pushed to the OLED. If you need more than 26 bits,
 * then the FIFO is not for you! Use shared memory and DMA in that case!
 *
 * Bit layout (total 32 bits):
 * - dest_id   : 3 bits  - destination core or handler identifier
 * - source_id : 3 bits  - origin/core identifier
 * - data      : 26 bits - small payload/command/index (not bulk data)
 */
typedef struct FIFOMessage {
    unsigned int dest_id : FIFO_DEST_ID_SIZE;
    unsigned int source_id : FIFO_SOURCE_ID_SIZE;
    unsigned int data : FIFO_DATA_SIZE;
} FIFOMessage;
bool construct_message(unsigned int dest_id, unsigned int source_id, unsigned int data, FIFOMessage* msg_out);
uint32_t message_to_uint32(FIFOMessage msg);
FIFOMessage message_from_uint32(uint32_t raw);


#ifndef RING_QUEUE_BUFFER_SIZE
#define RING_QUEUEBUFFER_SIZE 256
#endif

#if (RING_QUEUE_BUFFER_SIZE & (RING_QUEUE_BUFFER_SIZE - 1)) != 0
#error "BUFFER_SIZE must be a power of 2"
#endif

#define RING_QUEUE_BUFFER_MASK (RING_QUEUE_BUFFER_SIZE - 1)

template <typename T, int N>
class RingQueue {
    private: 
        T data[N];
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
        T* get_full_buffer();

        T pop_front(bool* result);
        bool push_back(const T value);
};


#if ENABLE_FIFO_MESSAGING == 1

/**
 * @brief Type for receiver callbacks registered with the FIFO router.
 *
 * Any ISR registered as a receiver handler must be non-blocking and
 * fast to execute. You should NEVER wait (block) inside a receiver ISR
 * because that will lock up the application. If the receiver needs to
 * perform long-running work, hand it off: set a flag, enqueue a work
 * item, or trigger a lower-priority/internal interrupt to perform the
 * work outside the ISR context.
 */
typedef void (*ReceiverTask)(FIFOMessage msg);

class ReceiverRouter {
    private:
        ReceiverTask handlers[FIFO_DEST_ID_MAX + 1];
        unsigned int registered_count = 0;
    public:
        bool register_handler(unsigned int id, ReceiverTask task);
        bool route_message(FIFOMessage msg);
        bool is_full();
        unsigned int get_registered_count();
};


#endif // ENABLE_FIFO_MESSAGING == 1

#include "ring_queue.tpp"

#endif