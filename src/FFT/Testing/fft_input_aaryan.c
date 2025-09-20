#include <stdlib.h>
#include <stdio.h>

#include "fft_funtions.h"

#define TIME_VIS_SYNC_1      300
#define TIME_VIS_SYNC_HOLD   10 + TIME_VIS_SYNC_1
#define TIME_VIS_SYNC_2      300 + TIME_VIS_SYNC_HOLD

#define TIME_START_BIT      30 + TIME_VIS_SYNC_2
#define TIME_VIS_CODE       30 * 8 + TIME_START_BIT
#define TIME_PARITY_BIT     30 + TIME_VIS_CODE
#define TIME_STOP_BIT       30 + TIME_PARITY_BIT

#define TIME_TRANSMISSION   1000 + TIME_STOP_BIT

#define FREQ_VIS_SYNC_1         1900
#define FREQ_VIS_SYNC_HOLD      1200 
#define FREQ_VIS_SYNC_2         1900

#define FREQ_START_BIT          1200
#define FREQ_VIS_HIGH           1300 
#define FREQ_VIS_LOW            1100 
#define FREQ_STOP_BIT           1200

#define FREQ_TRANSMISSION       1000

uint32_t time_global = 0;

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