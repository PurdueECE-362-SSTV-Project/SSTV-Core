#ifndef BMMPI_HPP
#define BMMPI_HPP
// BMMPI stands for Bare Metal MPI


#include <limits>
#include "pico/stdlib.h"
#include "pico/sync.h"

#include "native/n_bmmpi.hpp"
#include "native/n_types.hpp"
#include "AppConfig.h"


#if ENABLE_FIFO_MESSAGING == 1


void setup_FIFO_queue_reg(unsigned int core_num);
extern ReceiverRouter fifo_queue_core0; // input to core 0
extern ReceiverRouter fifo_queue_core1; // input to core 1

void fifo_core_0_irq();
void fifo_core_1_irq();

void init_fifo_core0();
void init_fifo_core1();


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


class SenderQueue : protected AtomicQueue<FIFOMessage> {
    private:
        RingQueue<FIFOMessage> internal_queue;
    protected:
        SenderQueue(int size);
        bool atomic_push_proc(FIFOMessage value) override;
        Optional<FIFOMessage> atomic_pop_proc() override;

        ~SenderQueue();
};


class ReceiverQueue: protected AtomicQueue<FIFOMessage> {
    private:
        RingQueue<RFIFOMessage> internal_queue; // index corresponds to target id
    protected:
        bool atomic_push_proc(FIFOMessage value) override;
        Optional<FIFOMessage> atomic_pop_proc() override;

        ~ReceiverQueue();
};

#include 

#endif // ENABLE_FIFO_MESSAGING == 1

#if ENABLE_SHARED_MEMORY == 1

#endif // ENABLE_SHARED_MEMORY == 1


#endif