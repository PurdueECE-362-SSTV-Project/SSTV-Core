#include "native/n_types.hpp"


template<typename T>
Optional<T>::Optional() : some(false) {}


template<typename T>
Optional<T>::Optional(T value) : some(true), value(value) {}


template<typename T>
T* Optional<T>::get_ref() {
    if(this->some) {
        return &this->value;
    }
    return nullptr;
}


template<typename T>
bool Optional<T>::is_some() {
    return this->some;
}


template<typename T>
T Optional<T>::take() {
    return this->value;
}