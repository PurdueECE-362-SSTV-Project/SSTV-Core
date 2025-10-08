#include <stdio.h>
#include "native/n_bmmpi.hpp"
#include "native/n_types.hpp"
#include "AppConfig.h"


template<typename T, int N>
BaseQueue<T, N>::BaseQueue(T default_value) : default_value(default_value) {}


template<typename T, int N>
bool BaseQueue<T, N>::full(int head, int tail) {
    int size = BaseQueue<T, N>::size(head, tail);
    return size == N;
}


template<typename T, int N>
bool BaseQueue<T, N>::empty(int head, int tail) {
    int size = BaseQueue<T, N>::size(head, tail);
    return size == 0;
}


template<typename T, int N>
int BaseQueue<T, N>::size(int head, int tail) {
    int size = ((tail - head) & (N - 1)) + (tail != head);
    return size;
}


template<typename T, int N>
int BaseQueue<T, N>::wraparound_increment(int current) {
    return (current + 1) & (N - 1);
}