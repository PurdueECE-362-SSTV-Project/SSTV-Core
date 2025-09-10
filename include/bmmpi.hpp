#ifndef BMMPI_HPP
#define BMMPI_HPP

#include <queue>


template <typename T, std::size_t N>
class RingQueue {
    private: 
        T data[N];
        std::size_t head = 0;
        std::size_t tail = 0;
        std::size_t count = 0;

    public:
        using value_type = T;
        using size_type = std::size_t;
        using reference = T&;
        using const_reference = const T&;

        reference front() { &this->data[head] }
};


#endif