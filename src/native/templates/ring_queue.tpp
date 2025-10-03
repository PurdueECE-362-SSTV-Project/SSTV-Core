#include <stdio.h>
#include "native/n_bmmpi.hpp"
#include "native/n_types.hpp"
#include "AppConfig.h"


template<typename T, int N>
BaseQueue<T, N>::BaseQueue(T default_value) : default_value(default_value) {}


template<typename T, int N>
int BaseQueue<T, N>::size() {
    return this->count;
}


template<typename T, int N>
int BaseQueue<T, N>::max_size() {
    return RING_QUEUE_BUFFER_SIZE;
}


template<typename T, int N>
bool BaseQueue<T, N>::empty() {
    return this->count == 0;
}


template<typename T, int N>
bool BaseQueue<T, N>::full() {
    return this->count == max_size();
}


template<typename T, int N>
int BaseQueue<T, N>::wraparound_increment(int current) {
    return (current + 1) & RING_QUEUE_BUFFER_MASK;
}


template<typename T, int N>
T BaseQueue<T, N>::pop_front(bool *result) {
    *result = true;
    if(this->empty()) {
        *result = false;
        return this->default_value;
    }
    T to_pop = this->pop_function();

    return to_pop;
}


template<typename T, int N>
bool BaseQueue<T, N>::push_back(const T value) {
    if(this->full()) {
        return false;
    }
    this->push_function(value);

    return true;
}


template<typename T, int N>
void RingQueue<T, N>::flush() {
    this->head = 0;
    this->tail = 0;
    this->count = 0;
}


template<typename T, int N>
int RingQueue<T, N>::size() {

}

this->data[this->head];

this->head = RingQueue::wraparound_increment(this->head);
this->count--;


this->data[this->tail] = value;

this->tail = RingQueue::wraparound_increment(this->tail);
this->count++;