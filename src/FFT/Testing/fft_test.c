#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "fft_funtions.h"

#define NFFT 1024 // Number of sampling points in the DFT
#define SRATE 11025 // Sampling Frequency 11025Hz 
# define M_PI           3.14159265358979323846
// #define WSIZE 10 // Winsow size in ms
// #define NWINDOW 

// Main function handles frequency input/output operations.
int main() {
    double *inputSignal; // The full input signal
    int signalLength = 4096; // n
    int hopSize = NFFT / 2; // H, 50% overlap test
    int numFrames = (signalLength - NFFT) / hopSize + 1;
    double window[NFFT]; // w
    
    double mags[NFFT/2 + 1];
    double max_mag = 0;
    int max_index = 0;

    // Signal allocation
    inputSignal = malloc(signalLength * sizeof(double));
    if (!inputSignal) {
        fprintf(stderr, "Failed to allocate inputSignal\n");
        return 1;
    }

    // Initialize inputSignal with data
    for (int i = 0; i < signalLength; i++) {
        inputSignal[i] = sin(2 * M_PI * 440.0 * i / SRATE); // 440 Hz tone example
    }

    // Initialize freqency array
    for (int k = 0; k < (NFFT/2 + 1); k++) {
        mags[k] = 0.0;
    }

    // Initialize Hann window
    for (int i = 0; i < NFFT; i++) {
        window[i] = 0.5 * (1 - cos(2 * M_PI * i / (NFFT - 1)));
    }
    
    // Allocate
    double *in_r = malloc(sizeof(double)*NFFT);
    kiss_fft_cpx *out_c = malloc(sizeof(kiss_fft_cpx)*(NFFT/2+1));
    kiss_fft_cfg cfg = kiss_fftr_alloc(NFFT, 0, 0, 0);


    for (int frame = 0; frame < numFrames; frame++) {
        int start = frame * hopSize;

        for(int i = 0; i < NFFT; i++) { // Windows frame
            in_r[i] = inputSignal[start + i] * window[i];
        }

        // Fill input (e.g., from audio capture)
        // for (int i = 0; i < NFFT; i++)
        //     in_r[i] = /* your real samples */ 0.0;

        // Execute FFT
        kiss_fftr(cfg, in_r, out_c);
        max_mag = 0;
        max_index = 0;

        // Process spectrum
        for (int k = 0; k <= NFFT/2; k++) {
            double freq = k * (double)SRATE / NFFT;
            mags[k]  = sqrt(out_c[k].r*out_c[k].r + out_c[k].i*out_c[k].i);
            if (mags[k] > max_mag) {
                max_mag = mags[k];
                max_index = k;
            }
        }
        printf("Frame %d max freq: %6.1f Hz with magnitude %f\n", frame, max_index * (double)SRATE / NFFT, max_mag);
    }
    
    
    // Cleanup
    kiss_fft_free(cfg);
    free(in_r);
    free(out_c);
    free(inputSignal);
    return 0;
}