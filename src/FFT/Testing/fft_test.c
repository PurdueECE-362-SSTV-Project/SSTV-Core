#include <stdlib.h>
#include <stdio.h>

#include "fft_funtions.h"

#define NFFT 1024
#define SRATE 44100

int main() {
    // Allocate
    double *in_r = malloc(sizeof(double)*NFFT);
    kiss_fft_cpx *out_c = malloc(sizeof(kiss_fft_cpx)*(NFFT/2+1));
    kiss_fft_cfg cfg = kiss_fftr_alloc(NFFT, 0, 0, 0);

    // Fill input (e.g., from audio capture)
    for (int i = 0; i < NFFT; i++)
        in_r[i] = /* your real samples */ 0.0;

    // Execute FFT
    kiss_fftr(cfg, in_r, out_c);

    // Process spectrum
    for (int k = 0; k <= NFFT/2; k++) {
        double freq = k * (double)SRATE / NFFT;
        double mag  = sqrt(out_c[k].r*out_c[k].r + out_c[k].i*out_c[k].i);
        printf("%6.1f Hz: %f\n", freq, mag);
    }

    // Cleanup
    kiss_fft_free(cfg);
    free(in_r);
    free(out_c);
    return 0;
}