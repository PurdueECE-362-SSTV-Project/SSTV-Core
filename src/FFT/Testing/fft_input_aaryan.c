#include "../../../lib/kissfft-master/kiss_fft.h"
#include "../../../lib/kissfft-master/kiss_fftr.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "fft_functions.h"

float time_global = 0;

static float gen_sin(float frequency) {
    //return (1.0 / ((float) INT64_MAX)) * sin(2.0 * M_PI * frequency * time_global / SFREQ); return (1.0 / ((float) INT64_MAX)) * sin(2.0 * M_PI * frequency * time_global / SFREQ); 
    printf("Test: %d\n", time_global);
    return sin(2.0 * M_PI * frequency * time_global / SFREQ); 
}

// // fft_input_custom_gen //
// time: Current Time (x(t))
// start_time: Where the SSTV signal starts
// float fft_input_custom_gen (uint32_t time, uint32_t start_time) {
//     // ret value
//     float output_val;

//     // Time difference (start) and updating global funtion for time
//     long long int time_n = time - start_time;
//     time_global = time;

//     if (time_n < 0) output_val = gen_sin(1);
//     else if (TIME_BOUNDS(time_n, 0, TIME_VIS_SYNC_1))                   output_val = gen_sin(FREQ_VIS_SYNC_1);
//     else if (TIME_BOUNDS(time_n, TIME_VIS_SYNC_1, TIME_VIS_SYNC_HOLD))  output_val = gen_sin(FREQ_VIS_SYNC_HOLD);
//     else if (TIME_BOUNDS(time_n, TIME_VIS_SYNC_HOLD, TIME_VIS_SYNC_2))  output_val = gen_sin(FREQ_VIS_SYNC_1);
//     else if (TIME_BOUNDS(time_n, TIME_VIS_SYNC_2, TIME_START_BIT))      output_val = gen_sin(FREQ_START_BIT);
//     else if (TIME_BOUNDS(time_n, TIME_START_BIT, TIME_VIS_CODE))        output_val = gen_sin(FREQ_VIS_HIGH);
//     else if (TIME_BOUNDS(time_n, TIME_VIS_CODE, TIME_PARITY_BIT))       output_val = gen_sin(FREQ_VIS_LOW);
//     else if (TIME_BOUNDS(time_n, TIME_PARITY_BIT, TIME_STOP_BIT))       output_val = gen_sin(FREQ_STOP_BIT);
//     else if (TIME_BOUNDS(time_n, TIME_STOP_BIT, TIME_TRANSMISSION))     output_val = gen_sin(FREQ_TRANSMISSION);
//     else output_val = 0.0;

//     return output_val;
// }

float fft_input_custom_gen (float time) {
    // ret value
    float output_val;

    // Time difference (start) and updating global funtion for time
    long long int time_n = time;
    time_global = time;

    if (TIME_BOUNDS(time_n, 0, 1024))                                output_val = gen_sin(1900);
    else if (TIME_BOUNDS(time_n, 1024, 2056))                         output_val = gen_sin(1200);
    else if (TIME_BOUNDS(time_n, 2056, 3200))                         output_val = gen_sin(1900);
    else if (TIME_BOUNDS(time_n, 3200, 4096))      output_val = gen_sin(1200);
    else if (TIME_BOUNDS(time_n, 640, 670))        output_val = gen_sin(1500);
    else if (TIME_BOUNDS(time_n, 670, 8000))       output_val = gen_sin(1100);
    else if (TIME_BOUNDS(time_n, TIME_PARITY_BIT, TIME_STOP_BIT))       output_val = gen_sin(1200);
    else if (TIME_BOUNDS(time_n, TIME_STOP_BIT, TIME_TRANSMISSION))     output_val = gen_sin(1500);
    else output_val = 0.0;

    return output_val;
}