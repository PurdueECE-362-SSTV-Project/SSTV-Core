#include "board/bmmpi.hpp"


template <typename T, size_t N, size_t W>
MerryMemoryOrigin<T, N, W>::MerryMemoryOrigin(T default_value) : MulticoreLocklessQueueSPSC<MerryBuffer<T, W>, N>(default_value) {
    for(int buffer_number = 0; buffer_number < N; buffer_number++) {
        unique_ptr<MerryBuffer<T, W>> buffer = std::make_unique<MerryBuffer<T, W>>();
        this->push_back(buffer);
    }
}


template <typename T, size_t N, size_t W>
bool call_task(PipelineTask<T, N, W> task, BaseQueue<T, N>* pull_queue, BaseQueue<T, N>* push_queue) {
    bool result;
    unique_ptr<MerryBuffer<T, W>> popped_value = pull_queue->pop_front(&result);

    if(!result) {
        return false; // task failed because no work is in queue
    }
    
    if(!task(popped_value)) {
        return false;
    }

    if(!push_queue->push_back(popped_value)) {
        DROP_COUNT++;
        return false;
    }

    return true;
}