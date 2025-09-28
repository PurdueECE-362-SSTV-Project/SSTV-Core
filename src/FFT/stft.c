#include "kiss_fft.h"
#include "kiss_fftr.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "fft_functions.h"

// Fucnctions:
float fft_input_custom_gen(float);
float* inputStore(int);
float* winInit(int);

//Allocs and store input data in pointer
float* inputStore(int signalLength) {
    float *inputSignal = malloc(signalLength * sizeof(float)); // TEMP: Produces an array to store a fixed sized, array for 10 seconds of data
    if (!inputSignal) {
        fprintf(stderr, "Failed to allocate inputSignal\n");
        return NULL; }

    for (int i = 0; i < signalLength; i++) {
        // int error = (rand() % 100) * (-1 + (rand() % 3));

        // inputSignal[i] = sin(2.0 * M_PI * (1900) * i / SFREQ); 
        inputSignal[i] = fft_input_custom_gen(i); 
    }
    return inputSignal; }

// Initializes window array
float* winInit(int samples) {
    float *window = malloc(samples * sizeof(float));
    if (!window) {
        fprintf(stderr, "Failed to allocate window\n");
        return NULL; }

    for (int i = 0; i < samples; i++) { // Hann window
        window[i] = 0.5 * (1 - cos(2 * M_PI * i / (samples - 1))); }
    return window; }

// Main function handles frequency input/output operations.
int main() {
    float *inputSignal;                              // The full input signal
    float sec = 1;                                  // Time in s
    int signalLength = SFREQ * sec;                 // Number of samples in the signal (n)
    //int NFFT = 256;                                // Length of each frame
    int hopLength = 54;                             // The distance between each FFT
    inputSignal = inputStore(signalLength);
    int numFrames = ((signalLength - NFFT) / hopLength) + 1; // Computes the number of FFT frames.
    float *window = winInit(NFFT);
    
    // printf("Frame length: %d\n", NFFT);
    // printf("Number of frames: %d\n", numFrames);
    // printf("Hop length: %d\n", hopLength);
    // printf("Number of frames: %d\n", numFrames);

    float mags[NFFT/2 + 1];
    float max_mag = 0;
    int max_index = 0;
    
    // Initialize freqency array
    for (int k = 0; k < (NFFT/2 + 1); k++) {
        mags[k] = 0.0;
    } 

    // Allocate
    float *in_r = malloc(sizeof(float)*NFFT);
    kiss_fft_cpx *out_c = malloc(sizeof(kiss_fft_cpx)*(NFFT/2+1));
    kiss_fftr_cfg cfg = kiss_fftr_alloc(NFFT, 0, 0, 0);

    for (int frame = 0; frame < numFrames; frame++) {
        int start = frame * hopLength;

        for(int i = 0; i < NFFT; i++) { // Windows frame
            int index = start + i;
            if (index < signalLength) {
                in_r[i] = inputSignal[index] * window[i];
            }
            else {
                in_r[i] = 0.0;
            }
        }

        // Execute FFT
        kiss_fftr(cfg, in_r, out_c);
        max_mag = 0;
        max_index = 0;

        // Process spectrum
        for (int k = 0; k <= NFFT/2; k++) {
            // float freq = k * (float)SFREQ / NFFT; 
            mags[k]  = sqrt(out_c[k].r*out_c[k].r + out_c[k].i*out_c[k].i);
            if (mags[k] > max_mag) {
                max_mag = mags[k];
                max_index = k;
            }
        }
        printf("%3.d: (%.5fs,  +%.5fs) max freq: %6.1f Hz with magnitude %f\n", 
            (frame + 1), // Frame being viewed
            (sec / numFrames * frame), // Time index of frame
            (sec / numFrames), // Time increment between frames
            max_index * (float)SFREQ / NFFT, // Max frequency of frame
            max_mag); // Max magnitude
    }  
    
    // Cleanup
    kiss_fft_free(cfg);
    free(in_r);
    free(out_c);
    free(inputSignal);
    return 0;
}