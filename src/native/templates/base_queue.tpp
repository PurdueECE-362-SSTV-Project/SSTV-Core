#include <stdio.h>
#include "native/n_bmmpi.hpp"
#include "native/n_types.hpp"
#include "AppConfig.h"


template<typename T, int N>
BaseQueue<T, N>::BaseQueue(T default_value) : default_value(default_value) {}


template<typename T, int N>
bool BaseQueue<T, N>::full(int current_head, int next_tail) {
    return current_head == next_tail;
}


template<typename T, int N>
bool BaseQueue<T, N>::empty(int current_head, int current_tail) {
    return current_head == current_tail;
}


template<typename T, int N>
int BaseQueue<T, N>::wraparound_increment(int current) {
    return (current + 1) & (N - 1);
}