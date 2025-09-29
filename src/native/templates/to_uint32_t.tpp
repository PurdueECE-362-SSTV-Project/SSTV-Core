#include "native/n_types.hpp"
#include <cstring> // for std::memcpy
#include <cstdint> // for uint32_t


template <typename T>
uint32_t to_uint32(T value) {
    static_assert(sizeof(T) <= 4, "to_uint32 can only be used with types of size 4 bytes or less");
    uint32_t result = 0;
    std::memcpy(&result, &value, sizeof(T));
    return result;
}


float to_float(uint32_t raw) {
    float result = 0.0f;
    std::memcpy(&result, &raw, sizeof(float));
    return result;
}