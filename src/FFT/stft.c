#include "../../lib/kissfft-master/kiss_fft.h"
#include "../../lib/kissfft-master/kiss_fftr.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "fft_functions.h"

// Main function handles frequency input/output operations.
int fft( int *inputSignal) {
    float mags[NFFT/2 + 1];
    float max_mag = 0;
    int max_index = 0;
    
    // Initialize freqency array
    for (int k = 0; k < (NFFT/2 + 1); k++) {
        mags[k] = 0.0;
    } 

    // Allocate
    // float *in_r = malloc(sizeof(float)*NFFT);
    kiss_fft_cpx *out_c = malloc(sizeof(kiss_fft_cpx)*(NFFT/2+1));
    kiss_fftr_cfg cfg = kiss_fftr_alloc(NFFT, 0, 0, 0);

    for (int frame = 0; frame < 512; frame++) {
        // Execute FFT
        kiss_fftr(cfg, inputSignal, out_c);
        max_mag = 0;
        max_index = 0;

        // Process spectrum
        for (int k = 0; k <= NFFT/2; k++) {
            mags[k]  = sqrt(out_c[k].r*out_c[k].r + out_c[k].i*out_c[k].i);
            if (mags[k] > max_mag) {
                max_mag = mags[k];
                max_index = k;
            }
        }
        // printf("%3.d: (%.3fs, +%.3fms) max freq: %6.1f Hz with magnitude %f\n", 
        //     (frame + 1), // Frame being viewed
        //     ((float)hopLength / SFREQ * frame), // Time index of frame
        //     ((float)hopLength / SFREQ * 1000), // Time increment between frames
        //     max_index * (float)SFREQ / NFFT, // Max frequency of frame
        //     max_mag); // Max magnitude
    }  
    
    // Cleanup
    kiss_fft_free(cfg);
    // free(in_r);
    free(out_c);
    return 0;
}

