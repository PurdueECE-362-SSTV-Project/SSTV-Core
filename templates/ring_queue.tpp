#include "bmmpi.hpp"
#include "types.hpp"


template<typename T>
RingQueue<T>::RingQueue(int size) : total_size(size) {}


template<typename T>
int RingQueue<T>::size() {
    return this->count;
}


template<typename T>
int RingQueue<T>::max_size() {
    return this->total_size;
}


template<typename T>
bool RingQueue<T>::empty() {
    return this->count == 0;
}


template<typename T>
bool RingQueue<T>::full() {
    return this->count == this->total_size;
}


template<typename T>
int RingQueue<T>::wraparound_increment(int current, int max) {
    int modulo_count = current % max;
    return (modulo_count) + (modulo_count != 0);
}


template<typename T>
Optional<T> RingQueue<T>::pop_front() {
    if(this->empty()) {
        return Optional();
    }
    T to_pop = this->data[this->front()];

    this->front = RingQueue::wraparound_increment(this->front, this->total_size);
    this->count--;
}


template<typename T>
Optional<T> RingQueue<T>::push_back(const T value) {
    if(this->full()) {
        return false;
    }

    this->back = RingQueue::wraparound_increment(this->back, this->total_size);
    this->count--;

    return true;
}