#include "../../../lib/kissfft-master/kiss_fft.h"
#include "../../../lib/kissfft-master/kiss_fftr.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "fft_functions.h"

// Fucnctions:
float fft_input_custom_gen(float);
float* inputStore(float*, int);
float* winInit(float*, int);

//Allocs and store input data in pointer
float* inputStore(float *signal, int length) {
    signal = malloc(length * sizeof(float)); // TEMP: Produces an array to store a fixed sized, array for 10 seconds of data
    if (!signal) {
        fprintf(stderr, "Failed to allocate inputSignal\n");
        return NULL; }

    for (int i = 0; i < length; i++) {
        int error = (rand() % 200) * (-1 + (rand() % 3));

        // signal[i] = sin(2.0 * M_PI * (1900 + error) * i / SFREQ); 
        //printf("Test: %d\n", i);
        // printf("Test: %d\n", error);
        signal[i] = fft_input_custom_gen(i); 
    }
    return &signal; }

// Initializes window array
float* winInit(float *window, int samples) {
    for (int i = 0; i < samples; i++) { // Hann window
        window[i] = 0.5 * (1 - cos(2 * M_PI * i / (samples - 1))); }
    return window;
}

// Main function handles frequency input/output operations.
int main() {
    float *inputSignal; // The full input signal
    float sec = 1; // (s)
    float signalLength = SFREQ * sec; // Number of samples in the signal (n)
    int nFrame = 2048; // Length of each frame
    int hopLength = nFrame / 2; // The distance between each FFT, H, 50% overlap test
    inputSignal = inputStore(inputSignal, signalLength);
    
    int numFrames = ((signalLength - nFrame) / hopLength) + 1; // Computes the number of FFT frames.
    printf("Frame length: %d\n", nFrame);
    printf("Number of frames: %d\n", numFrames);
    printf("Hop length: %d\n", hopLength);
    // printf("Number of frames: %d\n", numFrames);
    // int numFrames = (len(*inputSignal) - NFFT) + 1; // Number of hops
    float window[nFrame]; // w
    winInit(window, nFrame);

    float mags[nFrame/2 + 1];
    float max_mag = 0;
    int max_index = 0;
    
    // Initialize freqency array
    for (int k = 0; k < (nFrame/2 + 1); k++) {
        mags[k] = 0.0;
    } 

    // Allocate
    float *in_r = malloc(sizeof(float)*nFrame);
    kiss_fft_cpx *out_c = malloc(sizeof(kiss_fft_cpx)*(nFrame/2+1));
    kiss_fftr_cfg cfg = kiss_fftr_alloc(nFrame, 0, 0, 0);

    for (int frame = 0; frame < numFrames; frame++) {
        int start = frame * hopLength;

        for(int i = 0; i < nFrame; i++) { // Windows frame
            in_r[i] = inputSignal[start + i] * window[i];
        }

        // Execute FFT
        kiss_fftr(cfg, in_r, out_c);
        max_mag = 0;
        max_index = 0;

        // Process spectrum
        for (int k = 0; k <= nFrame/2; k++) {
            // float freq = k * (float)SFREQ / NFFT; 
            mags[k]  = sqrt(out_c[k].r*out_c[k].r + out_c[k].i*out_c[k].i);
            if (mags[k] > max_mag) {
                max_mag = mags[k];
                max_index = k;
            }
        }
        printf("%2.d: (%.1fs,  +%.2fs) max freq: %6.1f Hz with magnitude %f\n", 
            (frame + 1), // Frame being viewed
            (sec / numFrames * frame), // Time index of frame
            (sec / numFrames), // Time increment between frames
            max_index * (float)SFREQ / nFrame, // Max frequency of frame
            max_mag); // Max magnitude
    }  
    
    // Cleanup
    kiss_fft_free(cfg);
    free(in_r);
    free(out_c);
    free(inputSignal);
    return 0;
}