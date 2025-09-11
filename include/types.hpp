#ifndef TYPES_HPP
#define TYPES_HPP


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


#endif