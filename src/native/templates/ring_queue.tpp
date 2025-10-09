#include "native/n_bmmpi.hpp"


template <typename T, int N>
RingQueue<T, N>::RingQueue(T default_value) : BaseQueue<T, N>(default_value) {}


template <typename T, int N>
void RingQueue<T, N>::flush() {
    this->head = 0;
    this->tail = 0;
    this->size = 0;
}


template <typename T, int N>
bool RingQueue<T, N>::push_back(const T value) {
    int next_tail = BaseQueue<T, N>::wraparound_increment(this->tail);
    if(this->size == N) {
        return false;
    }

    this->data[this->tail] = value;
    this->tail = next_tail;
    this->size++;

    return true;
}


template <typename T, int N>
T RingQueue<T, N>::pop_front(bool *result) {
    *result = true;
    if(this->size == 0) {
        *result = false;
        return this->default_value;
    }

    T data_value = this->data[this->head];
    this->head = BaseQueue<T, N>::wraparound_increment(this->head);
    this->size--;

    return data_value;
}