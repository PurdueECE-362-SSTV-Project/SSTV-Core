#include "board/bmmpi.hpp"


template <typename T, int N>
void MulticoreLocklessQueue<T, N>::flush() {
    void
}


template <typename T, int N>
bool MulticoreLocklessQueue<T, N>::push_back(const T value) {
    if(BaseQueue<T, N>::full(this->head.load(), this->tail.load())) {return false;}
    unsigned int old_tail = this->tail.load();
    unsigned int new_tail = BaseQueue<T, N>::wraparound_increment(old_tail);

    while(!this->tail.compare_exchange_weak(old_tail, new_tail)) {
        old_tail = this->tail.load();
        new_tail = BaseQueue<T, N>::wraparound_increment(old_tail);
        if(BaseQueue<T, N>::full(this->head.load(), this->tail.load())) {return false;}
    }

    this->data[new_tail] = value;

    return true;
}


template <typename T, int N>
T MulticoreLocklessQueue<T, N>::pop_front(bool *result) {
    *result = true;
    if(BaseQueue<T, N>::empty(this->head.load(), this->tail.load())) {
        *result = false;
        return this->default_value;
    }
    unsigned int old_head = this->head.load(memory_order_acquire);
    unsigned int new_head = BaseQueue<T, N>::wraparound_increment(old_head);

    while(!this->head.compare_exchange_weak(old_head, new_head, memory_order_acquire)) {
        old_head = this->head.load(memory_order_acquire);
        new_head = BaseQueue<T, N>::wraparound_increment(old_head);

        if(BaseQueue<T, N>::empty(this->head.load(), this->tail.load())) {
            *result = false;
            return this->default_value;
        }
    }

    T return_value = this->data[old_head];
    this->head.load(memory_order_release);

    return return_value;
}