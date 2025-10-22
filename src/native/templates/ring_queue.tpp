#include <stdio.h>
#include "native/n_bmmpi.hpp"
#include "native/n_types.hpp"
#include "AppConfig.h"


template<typename T>
RingQueue<T>::RingQueue(T default_value) : default_value(default_value) {}


template<typename T>
int RingQueue<T>::size() {
    return this->count;
}


template<typename T>
int RingQueue<T>::max_size() {
    return RING_QUEUE_BUFFER_SIZE;
}


template<typename T>
bool RingQueue<T>::empty() {
    return this->count == 0;
}


template<typename T>
bool RingQueue<T>::full() {
    return this->count == max_size();
}


template<typename T>
int RingQueue<T>::wraparound_increment(int current) {
    return (current + 1) & RING_QUEUE_BUFFER_MASK;
}


template<typename T>
T RingQueue<T>::pop_front() {
    if(this->empty()) {
        return this->default_value;
    }
    T to_pop = this->data[this->head];

    this->head = RingQueue::wraparound_increment(this->head);
    this->count--;

    return to_pop;
}


template<typename T>
bool RingQueue<T>::push_back(const T value) {
    if(this->full()) {
        return false;
    }
    this->data[this->tail] = value;

    this->tail = RingQueue::wraparound_increment(this->tail);
    this->count++;

    return true;
}


template<typename T>
void RingQueue<T>::flush() {
    this->head = 0;
    this->tail = 0;
    this->count = 0;
}