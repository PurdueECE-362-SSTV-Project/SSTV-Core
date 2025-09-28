#ifndef _FFT_FUNTIONS
#define _FFT_FUNTIONS

#include "kiss_fft.h"
#include "kiss_fftr.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define M_PI        3.14159265358979323846

#define NFFT        256         // Number of sampling points in the DFT
#define SFREQ       11025.0             // Sampling Frequency 11025Hz

#define TIME_BOUNDS(time, lower_bound, upper_bound)   ((time >= lower_bound) && (time < upper_bound))

#define TIME_VIS_SYNC_1      (300 * 1e-3 * SFREQ)
#define TIME_VIS_SYNC_HOLD   (10 * 1e-3 * SFREQ) + TIME_VIS_SYNC_1 
#define TIME_VIS_SYNC_2      (300 * 1e-3 * SFREQ) + TIME_VIS_SYNC_HOLD

#define TIME_START_BIT      (30 * 1e-3 * SFREQ) + TIME_VIS_SYNC_2
#define TIME_VIS_CODE       (30 * 8 * 1e-3 * SFREQ) + TIME_START_BIT
#define TIME_PARITY_BIT     (30 * 1e-3 * SFREQ) + TIME_VIS_CODE
#define TIME_STOP_BIT       (30 * 1e-3 * SFREQ) + TIME_PARITY_BIT

#define TIME_TRANSMISSION   (1000 * 1e-3 * SFREQ) + TIME_STOP_BIT

#define FREQ_VIS_SYNC_1         1900
#define FREQ_VIS_SYNC_HOLD      1200 
#define FREQ_VIS_SYNC_2         1900

#define FREQ_START_BIT          1200
#define FREQ_VIS_HIGH           1300 
#define FREQ_VIS_LOW            1100 
#define FREQ_STOP_BIT           1200

#define FREQ_TRANSMISSION       1100

#endif
