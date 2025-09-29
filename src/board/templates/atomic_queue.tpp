#include "board/bmmpi.hpp"


template<typename T>
bool AtomicQueue<T>::atomic_push(T value) {
    critical_section_enter_blocking(&this->at_queue_cs);
    bool result = this->atomic_push_proc(value);
    critical_section_exit(&this->at_queue_cs);

    return result;
}


template<typename T>
T AtomicQueue<T>::atomic_pop() {
    critical_section_enter_blocking(&this->at_queue_cs);
    T result = this->atomic_pop_proc();
    critical_section_exit(&this->at_queue_cs);

    return result;
}


template<typename T>
bool AtomicQueue<T>::is_full() {
    critical_section_enter_blocking(&this->at_queue_cs);
    bool result = this->internal_queue.is_full();
    critical_section_exit(&this->at_queue_cs);

    return result;
}


template<typename T>
bool AtomicQueue<T>::is_empty() {
    critical_section_enter_blocking(&this->at_queue_cs);
    bool result = this->internal_queue.is_empty();
    critical_section_exit(&this->at_queue_cs);

    return result;
}