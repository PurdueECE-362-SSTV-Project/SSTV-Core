#include "board/bmmpi.hpp"


template <typename T, size_t N>
InterruptQueue<T, N>::InterruptQueue(T default_value) : RingQueue<T, N>(default_value) {
    critical_section_init(&this->at_queue_cs);
}


template <typename T, size_t N>
void InterruptQueue<T, N>::flush() {
    critical_section_enter_blocking(&this->at_queue_cs);
    RingQueue<T, N>::flush();
    critical_section_exit(&this->at_queue_cs);
}


template <typename T, size_t N>
bool InterruptQueue<T, N>::push_back(const T value) {
    critical_section_enter_blocking(&this->at_queue_cs);
    bool result = RingQueue<T, N>::push_back(value);
    critical_section_exit(&this->at_queue_cs);

    return result;
}


template <typename T, size_t N>
T InterruptQueue<T, N>::pop_front(bool *result) {
    critical_section_enter_blocking(&this->at_queue_cs);
    T data_value = RingQueue<T, N>::pop_front(result);
    critical_section_exit(&this->at_queue_cs);

    return data_value;
}