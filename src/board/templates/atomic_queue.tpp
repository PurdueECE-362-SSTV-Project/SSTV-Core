#include "board/bmmpi.hpp"


template<typename T, int N>
bool AtomicQueue<T, N>::atomic_push(T value) {
    critical_section_enter_blocking(&this->at_queue_cs);
    bool result = this->internal_queue.push(value);
    critical_section_exit(&this->at_queue_cs);

    return result;
}


template<typename T, int N>
T AtomicQueue<T, N>::atomic_pop() {
    critical_section_enter_blocking(&this->at_queue_cs);
    T result = this->internal_queue.pop();
    critical_section_exit(&this->at_queue_cs);

    return result;
}


template<typename T, int N>
bool AtomicQueue<T, N>::is_full() {
    critical_section_enter_blocking(&this->at_queue_cs);
    bool result = this->internal_queue.is_full();
    critical_section_exit(&this->at_queue_cs);

    return result;
}


template<typename T, int N>
bool AtomicQueue<T, N>::is_empty() {
    critical_section_enter_blocking(&this->at_queue_cs);
    bool result = this->internal_queue.is_empty();
    critical_section_exit(&this->at_queue_cs);

    return result;
}


template<typename T, int N>
T* AtomicQueue<T, N>acquire_queue_array() {
    critical_section_enter_blocking(&this->at_queue_cs);
    return this->internal_queue.get_full_buffer();
}


template<typename T, int N>
bool AtomicQueue<T, N>release_queue_array(T* queue_array) {
    if(queue_array != &self.internal_queue[0]) {
        return false;
    }
    critical_section_exit(&this->at_queue_cs);

    return true;
}