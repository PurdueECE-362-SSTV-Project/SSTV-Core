#ifndef I_Q_ANALYSIS_H
#define I_Q_ANALYSIS_H

#include <stdint.h>

#define SAMPLE_RATE 15000
#define PI 3.14159265359

// --- Half-band filter ---
typedef struct {
    int16_t bufi[64];
    int16_t bufq[64];
    uint8_t pointer;
} half_band_filter;

void filter_init(half_band_filter *);
void filter(half_band_filter *, int16_t *, int16_t *);

void decode_iq(int16_t, int16_t, int16_t *);
void decode_audio(int16_t, int16_t *);

// --- CORDIC ---
#define CORDIC_ITER 16

void cordic_init(void);
void cordic_rectangular_to_polar(int16_t i, int16_t q, uint16_t *mag, int16_t *phase);

// --- SSB Demod ---
extern half_band_filter ssb_filter;
extern uint8_t ssb_phase;
extern int16_t last_phase;
extern int32_t smoothed_sample;

#endif // I_Q_ANALYSIS_H