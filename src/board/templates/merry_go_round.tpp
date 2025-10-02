#include "board/bmmpi.hpp"


template <typename T, int N, std::size_t W>
MerryMemoryOrigin<T, N, W>::MerryMemoryOrigin() {
    for(int buffer_number = 0; buffer_number < N; buffer_number++) {
        std::unique_ptr<MerryBuffer<T, W>> buffer = std::make_unique<MerryBuffer<T, W>>();
        this->atomic_push(buffer);
    }
}


template <typename T, int N, std::size_t W>
bool MerryTask<T, N, W>::call_synchronous() {
    bool result;
    unique_ptr<MerryBuffer<T, W>> popped_value = inbound_queue.atomic_pop(&result);

    if(!result) {
        return false; // task failed because no work is in queue
    }
    
    if(!this->function.run(popped_value)) {
        return false;
    }

    this->outbound_queue.atomic_push(popped_value);
    return true;
}



