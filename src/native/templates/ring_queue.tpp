#include <stdio.h>
#include "native/n_bmmpi.hpp"
#include "native/n_types.hpp"
#include "AppConfig.h"


template<typename T, int N>
RingQueue<T, N>::RingQueue(T default_value) : default_value(default_value) {}


template<typename T, int N>
int RingQueue<T, N>::size() {
    return this->count;
}


template<typename T, int N>
int RingQueue<T, N>::max_size() {
    return RING_QUEUE_BUFFER_SIZE;
}


template<typename T, int N>
bool RingQueue<T, N>::empty() {
    return this->count == 0;
}


template<typename T, int N>
bool RingQueue<T, N>::full() {
    return this->count == max_size();
}


template<typename T, int N>
int RingQueue<T, N>::wraparound_increment(int current) {
    return (current + 1) & RING_QUEUE_BUFFER_MASK;
}


template<typename T, int N>
T* RingQueue<T, N>::get_full_buffer() {
    return &this->data[0];
}


template<typename T, int N>
T RingQueue<T, N>::pop_front(bool *result) {
    *result = true;
    if(this->empty()) {
        *result = false;
        return this->default_value;
    }
    T to_pop = this->data[this->head];

    this->head = RingQueue::wraparound_increment(this->head);
    this->count--;

    return to_pop;
}


template<typename T, int N>
bool RingQueue<T, N>::push_back(const T value) {
    if(this->full()) {
        return false;
    }
    this->data[this->tail] = value;

    this->tail = RingQueue::wraparound_increment(this->tail);
    this->count++;

    return true;
}


template<typename T, int N>
void RingQueue<T, N>::flush() {
    this->head = 0;
    this->tail = 0;
    this->count = 0;
}