#ifndef _FFT_FUNTIONS
#define _FFT_FUNTIONS

#define BUFFER_SIZE     256             // Number of samples in the buffer

// Structure for queue node and the information it holds
struct linked_queue_t {
    struct linked_queue_t* prev_node;
    void* buffer_adcOut;
    void* buffer_fftOut;
    struct linked_queue_t* next_node;
};

// Funtion Declerations
void queue_init (uint8_t);

#endif