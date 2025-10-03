#include <stdio.h>
#include "native/n_bmmpi.hpp"
#include "native/n_types.hpp"
#include "AppConfig.h"


template<typename T, int N>
BaseQueue<T, N>::BaseQueue(T default_value) : default_value(default_value) {}


template<typename T, int N>
bool BaseQueue<T, N>::full(int head, int tail) {
    return BaseQueue::size(head, tail, N) == max_size;
}


template<typename T, int N>
bool BaseQueue<T, N>::empty(int head, int tail) {
    return BaseQueue::size(head, tail, N) == 0;
}


template<typename T, int N>
int BaseQueue<T, N>::size(int head, int tail) {
    if(tail > head) {
        return tail - head;
    }
    else {
        return (N - head) + tail + 1
    }
}


template<typename T, int N>
int BaseQueue<T, N>::wraparound_increment(int current) {
    return (current + 1) & RING_QUEUE_BUFFER_MASK;
}