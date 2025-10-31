#include "../../lib/kissfft-master/kiss_fft.h"
#include "../../lib/kissfft-master/kiss_fftr.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>

#include "fft_functions.h"


#define M 63 
#define L (2*M) + 1 // Discrete length kernel

// Computes the convolution of two input arrays
void conv(float *x, float *h, float *y) {
    // x and h are input arrays
    // y is the calculated convultion
    size_t lenx = sizeof(x) / sizeof(*x);
    size_t lenh = sizeof(y) / sizeof(*y);   
    int l = lenx + lenh - 1; 

    for (int i = 0; i < l; i ++) {
        y[i] = 0.0;
        for(int n = 0; n < l; n++) {
            if ((i - n >= 0) && (n - i <= lenh)) {
                y[n] = y[n] + x[i] * h[n - i];
            }
        }
    }
}

// Initializes window array
float* winInit(int samples) {
    float *window = malloc(samples * sizeof(float));
    if (!window) {
        fprintf(stderr, "Failed to allocate window\n");
        return NULL; }

    for (int i = 0; i < samples; i++) { // Hann window
        window[i] = 0.5 * (1 - cos(2 * M_PI * i / (samples - 1))); }
return window; }


// Takes an input signal x and compes the
// Hilbert transform and stores it in q
void hilbert(float x, float *q) {
    float h[L];

    for (int i = 0; i < L; i++) {
        int m = i - M; // Shifted sample index
        if (m == 0) h[i] = 0.0;
        else if (m % 2 != 0) {
            h[i] = (2 / (M_PI * (double)(m - M))); // Applies DHT to odd indicies
        }
        else if (m % 2 == 0) h[i] = 0.0;
    }
    float *w = winInit(L); // Creates a Hann window
    float hw[L]; // Windowed Hilbert array

    for(int i = 0; i < L; i++) { // Windows frame
        hw[i] = h[i] * w[i];
    }
    conv(&x, &h, &q); // Convolves the Hilbert window with x
}