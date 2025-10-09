#include "board/bmmpi.hpp"


template <typename T, size_t N>
MulticoreLocklessQueueSPSC<T, N>::MulticoreLocklessQueueSPSC(T default_value) : BaseQueue<T, N>(default_value) {
    critical_section_init(&this->at_queue_cs);
}


template <typename T, size_t N>
void MulticoreLocklessQueueSPSC<T, N>::flush() {
    bool result;

    while(BaseQueue<T, N>::size(
        this->head.load(memory_order_relaxed),
        this->tail.load(memory_order_acquire)
    ) > 0) {
        this->pop_front(&result);
    }
}


template <typename T, size_t N>
bool MulticoreLocklessQueueSPSC<T, N>::push_back(const T value) {
    size_t const current_tail = this->tail.load(memory_order_relaxed);
    size_t const next_tail = BaseQueue<T, N>::wraparound_increment(current_tail);

    if(next_tail == this->head.load(memory_order_acquire)) {
        return false;
    }

    this->data[next_tail] = value;
    this->tail.store(next_tail, memory_order_release);

    return true;
}


template <typename T, size_t N>
T MulticoreLocklessQueueSPSC<T, N>::pop_front(bool *result) {
    size_t const current_head = this->head.load(memory_order_relaxed);

    if(current_head == this->tail.load(memory_order_acquire)) {
        *result = false;
        return this->default_value;
    }

    size_t const next_head = BaseQueue<T, N>::wraparound_increment(current_head);
    T return_value = this->data[current_head];
    this->head.store(next_head, memory_order_release);

    *result = true;
    return return_value;
}