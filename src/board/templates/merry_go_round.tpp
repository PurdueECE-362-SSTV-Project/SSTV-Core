#include "board/bmmpi.hpp"


template <typename T, size_t N, size_t W>
MerryMemoryOrigin<T, N, W>::MerryMemoryOrigin() {
    for(int buffer_number = 0; buffer_number < N; buffer_number++) {
        unique_ptr<MerryBuffer<T, W>> buffer = std::make_unique<MerryBuffer<T, W>>();
        this->push_back(buffer);
    }
}


template <typename T, size_t N, size_t W>
MerryTask<T, N, W>::MerryTask(
    MerryTaskFunction<T, W> function, 
    TaskManagerFunction set_up, 
    TaskManagerFunction tear_down,
    char core
) : set_up(set_up), tear_down(tear_down), function(function), core(core) {}


template <typename T, size_t N, size_t W>
bool MerryTask<T, N, W>::call() {
    bool result;
    unique_ptr<MerryBuffer<T, W>> popped_value = inbound_queue.pop_front(&result);

    if(!result) {
        return false; // task failed because no work is in queue
    }
    
    if(!this->function.run(popped_value)) {
        return false;
    }

    if(!this->outbound_queue.push_back(popped_value)) {
        this->loss_counter++;
    }
    return true;
}


template<typename T, size_t N, size_t W>
bool MerryTaskBuilder<T, N, W>::register_irq_task(
    MerryTaskFunction<T, W> function, 
    TaskManagerFunction set_up, 
    TaskManagerFunction tear_down, 
    MerryTaskInterrupt interrupt,
    char core
) {
    if(this->irq_tasks[interrupt.irq_num] != nullptr) {
        return false;
    }
    unique_ptr<MerryTask<T, N, W>> task = make_unique<MerryTask<T, N, W>>(function, set_up, tear_down, core);
    task.
    this->irq_tasks[interrupt.irq_num] = task;
    GENERATE_IRQ_FUNCTION((interrupt.irq_num), interrupt.acknowledge, T, N, W);
    return true;
}


template<typename T, size_t N, size_t W>
unique_ptr<MerryTask<T, N, W>> MerryTaskBuilder<T, N, W>::register_main_loop_task(
    MerryTaskFunction<T, W> function, 
    TaskManagerFunction set_up, 
    TaskManagerFunction tear_down, 
    char core
) {
    return make_unique<MerryTask<T, N, W>>(function, set_up, tear_down, core);
}

