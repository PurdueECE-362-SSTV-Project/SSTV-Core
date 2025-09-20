#ifndef _FFT_FUNTIONS
#define _FFT_FUNTIONS

#include "../lib/kissfft-master/kiss_fft.h"
#include "../lib/kissfft-master/kiss_fftr.h"
#include <stdlib.h>
#include <stdio.h>

#define SAMPLING_FREQUENCY  11000   // 11 k

// // Bool for conditional where time is double bounded //
// time: Time
// start_time: time > start_time
// end_time: time < end_time
#define TIME_BOUNDS(time, start_time, end_time) bool (time > start_time && time < end_time)

#endif
