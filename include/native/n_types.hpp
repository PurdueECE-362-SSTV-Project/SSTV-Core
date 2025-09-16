#ifndef N_TYPES_HPP
#define N_TYPES_HPP


template <typename T>
class Optional {
    private:
        T value;
        bool some = false;
    public:
        Optional();
        Optional(T value);

        T* get_ref();
        bool is_some();
        T take();
};


#include "optional.tpp"


#endif