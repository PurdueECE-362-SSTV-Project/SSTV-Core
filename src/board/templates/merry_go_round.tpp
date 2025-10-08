#include "board/bmmpi.hpp"


template <typename T, size_t N, size_t W>
MerryMemoryOrigin<T, N, W>::MerryMemoryOrigin() {
    for(int buffer_number = 0; buffer_number < N; buffer_number++) {
        unique_ptr<MerryBuffer<T, W>> buffer = std::make_unique<MerryBuffer<T, W>>();
        this->push_back(buffer);
    }
}