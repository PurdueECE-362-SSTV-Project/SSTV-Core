#include "native/n_bmmpi.hpp"


template <typename T, int N>
void RingQueue<T, N>::flush() {
    this->head = 0;
    this->tail = 0;
}


template <typename T, int N>
bool RingQueue<T, N>::push_back(const T value) {
    if(BaseQueue<T, N>::full(this->head, this->tail)) {
        return false;
    }

    this->data[this->tail] = value;
    this->tail = BaseQueue<T, N>::wraparound_increment(this->tail);

    return true;
}


template <typename T, int N>
T RingQueue<T, N>::pop_front(bool *result) {
    if(BaseQueue<T, N>::empty(this->head, this->tail)) {
        *result = false;
        return this->default_value;
    }

    T data_value = this->data[this->head];
    this->head = BaseQueue<T, N>::wraparound_increment(this->tail);

    return data_value;
}