#include <stdlib.h>
#include <stdio.h>

#include "fft_funtions.h"

uint32_t time_global = 0;

// int TIME_BOUNDS(uint32_t, uint32_t, uint32_t);

static double gen_sin(uint32_t frequency) {
        return sin(time_global * frequency);
}

// // fft_input_custom_gen //
// time: Current Time (x(t))
// start_time: Where the SSTV signal starts
double fft_input_custom_gen (uint32_t time, uint32_t start_time) {
    // ret value
    double output_val;

    // Time difference (start) and updating global funtion for time
    uint32_t time_n = time - start_time;
    time_global = time;
    
    if (time_n > 0)         output_val = gen_sin(1);
    else if (TIME_BOUNDS(time_n, 0, TIME_VIS_SYNC_1))                    output_val = gen_sin(FREQ_VIS_SYNC_1);
    else if (TIME_BOUNDS(time_n, TIME_VIS_SYNC_1, TIME_VIS_SYNC_HOLD))   output_val = gen_sin(FREQ_VIS_SYNC_HOLD);


} 