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

// TIME DURATION LIST
#define TIME_VIS_SYNC_1      300
#define TIME_VIS_SYNC_HOLD   10 + TIME_VIS_SYNC_1
#define TIME_VIS_SYNC_2      300 + TIME_VIS_SYNC_HOLD

#define TIME_START_BIT      30 + TIME_VIS_SYNC_2
#define TIME_VIS_CODE       30 * 8 + TIME_START_BIT
#define TIME_PARITY_BIT     30 + TIME_VIS_CODE
#define TIME_STOP_BIT       30 + TIME_PARITY_BIT

#define TIME_TRANSMISSION   1000 + TIME_STOP_BIT

// FREQUENCY LIST
#define FREQ_VIS_SYNC_1         1900
#define FREQ_VIS_SYNC_HOLD      1200 
#define FREQ_VIS_SYNC_2         1900

#define FREQ_START_BIT          1200
#define FREQ_VIS_HIGH           1300 
#define FREQ_VIS_LOW            1100 
#define FREQ_STOP_BIT           1200

#define FREQ_TRANSMISSION       1000

#endif
