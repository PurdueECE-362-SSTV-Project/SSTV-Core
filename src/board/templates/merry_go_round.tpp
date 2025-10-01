#include "board/bmmpi.hpp"


template <typename T, int N, int W>
MerryMemoryOrigin<T, N, W>::MerryMemoryOrigin() {
    for(int buffer_number = 0; buffer_number < N; buffer_number++) {
        T* buffer = (T*)calloc(W, sizeof(T));
        this->origin_queue.atomic_push(buffer);
    }
}


template <typename T, int N, int W>
MerryMemoryOrigin<T, N, W>::~MerryMemoryOrigin() {
    T* queue_array = this->origin_queue.acquire_queue_array();
    for(int buffer_number = 0; buffer_number < N; buffer_number++) {
        T[buffer_number]
    }
}
