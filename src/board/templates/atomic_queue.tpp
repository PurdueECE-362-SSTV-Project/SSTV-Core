#include "board/bmmpi.hpp"


template <typename T, size_t N>
MulticoreLocklessQueueSPSC<T, N>::MulticoreLocklessQueueSPSC(T default_value) : BaseQueue<T, N>(default_value) {
    critical_section_init(&this->at_queue_cs);
}


template <typename T, size_t N>
void MulticoreLocklessQueueSPSC<T, N>::flush() {
    critical_section_enter_blocking(&this->at_queue_cs);
    
    this->head.store(0, memory_order_relaxed);
    this->tail.store(0, memory_order_relaxed);
    this->count.store(0, memory_order_relaxed);

    critical_section_exit(&this->at_queue_cs);
}


template <typename T, size_t N>
bool MulticoreLocklessQueueSPSC<T, N>::push_back(const T value) {
    size_t const current_tail = this->tail.load(memory_order_relaxed);
    size_t const next_tail = BaseQueue<T, N>::wraparound_increment(current_tail);

    size_t start_count = this->count.load(memory_order_acquire);
    size_t new_count = start_count + 1;

    if(N < new_count) {
        return false;
    }

    this->tail.store(next_tail, memory_order_release);
    this->data[current_tail] = value;
    this->count.store(new_count, memory_order_release);

    return true;
}


template <typename T, size_t N>
T MulticoreLocklessQueueSPSC<T, N>::pop_front(bool *result) {
    size_t const current_head = this->head.load(memory_order_relaxed);
    size_t start_count = this->count.load(memory_order_acquire);

    if(start_count == 0) {
        *result = false;
        return this->default_value;
    }

    size_t const next_head = BaseQueue<T, N>::wraparound_increment(current_head);

    this->head.store(next_head, memory_order_release);
    T return_value = this->data[current_head];
    this->count.store(start_count - 1, memory_order_release);

    *result = true;

    return return_value;
}