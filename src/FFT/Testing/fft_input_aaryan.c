#include <stdlib.h>
#include <stdio.h>

#include "fft_funtions.h"

uint32_t time_global = 0;

static double gen_sin(uint32_t frequency)   return sin(time_global * frequency);

// // fft_input_custom_gen //
// time: Current Time (x(t))
// start_time: Where the SSTV signal starts
double fft_input_custom_gen (uint32_t time, uint32_t start_time) { 
    // ret value
    double output_val;

    // Time difference (start) and updating global funtion for time
    uint32_t time_n = time - start_time;
    time_global = time;
    
    if (time_n > 0) output_val = gen_sin(1);
    else if (TIME_BOUNDS(time_n, 0, TIME_VIS_SYNC_1))                       output_val = gen_sin(FREQ_VIS_SYNC_1);
    else if (TIME_BOUNDS(time_n, TIME_VIS_SYNC_1, TIME_VIS_SYNC_HOLD))      output_val = gen_sin(FREQ_VIS_SYNC_HOLD);
    else if (TIME_BOUNDS(time_n, TIME_VIS_SYNC_HOLD, TIME_VIS_SYNC_2))      output_val = gen_sin(FREQ_VIS_SYNC_1);
    else if (TIME_BOUNDS(time_n, TIME_VIS_SYNC_2, TIME_START_BIT))          output_val = gen_sin(FREQ_START_BIT);
    else if (TIME_BOUNDS(time_n, TIME_START_BIT, TIME_VIS_CODE))            output_val = gen_sin(FREQ_VIS_HIGH);
    else if (TIME_BOUNDS(time_n, TIME_VIS_CODE, TIME_PARITY_BIT))           output_val = gen_sin(FREQ_VIS_LOW);
    else if (TIME_BOUNDS(time_n, TIME_PARITY_BIT, TIME_STOP_BIT))           output_val = gen_sin(FREQ_STOP_BIT);
    else if (TIME_BOUNDS(time_n, TIME_STOP_BIT, TIME_TRANSMISSION))         output_val = gen_sin(FREQ_TRANSMISSION);
    else     output_val = 0.0;                  // fallback or silence outside defined time range

    return output_val;
} 