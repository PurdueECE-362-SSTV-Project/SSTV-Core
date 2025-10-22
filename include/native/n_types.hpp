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


template <typename T>
uint32_t to_uint32(T value);


float to_float(uint32_t raw);


#include "optional.tpp"


#endif