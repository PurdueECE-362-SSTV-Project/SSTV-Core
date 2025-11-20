#include <math.h>

// Custom
#include "I_Q_analysis.h"

// --- Half-band filter implementation ---
void filter_init(half_band_filter *f) {
    f->pointer = 0;
    for(int i=0;i<64;i++) f->bufi[i]=f->bufq[i]=0;
}

void filter(half_band_filter *f, int16_t *i, int16_t *q) {
    f->bufi[f->pointer] = *i;
    f->bufq[f->pointer] = *q;
    f->pointer = (f->pointer + 1) & 0x3f;  // FIXED: Combined operation

    const uint8_t idx2 = (f->pointer + 2) & 0x3f;
    const uint8_t idx4 = (f->pointer + 4) & 0x3f;
    const uint8_t idx6 = (f->pointer + 6) & 0x3f;
    const uint8_t idx8 = (f->pointer + 8) & 0x3f;
    const uint8_t idx10 = (f->pointer + 10) & 0x3f;
    const uint8_t idx12 = (f->pointer + 12) & 0x3f;
    const uint8_t idx14 = (f->pointer + 14) & 0x3f;
    const uint8_t idx16 = (f->pointer + 16) & 0x3f;
    const uint8_t idx18 = (f->pointer + 18) & 0x3f;
    const uint8_t idx20 = (f->pointer + 20) & 0x3f;
    const uint8_t idx22 = (f->pointer + 22) & 0x3f;
    const uint8_t idx24 = (f->pointer + 24) & 0x3f;
    const uint8_t idx26 = (f->pointer + 26) & 0x3f;
    const uint8_t idx28 = (f->pointer + 28) & 0x3f;
    const uint8_t idx30 = (f->pointer + 30) & 0x3f;
    const uint8_t idx31 = (f->pointer + 31) & 0x3f;
    const uint8_t idx32 = (f->pointer + 32) & 0x3f;
    const uint8_t idx34 = (f->pointer + 34) & 0x3f;
    const uint8_t idx36 = (f->pointer + 36) & 0x3f;
    const uint8_t idx38 = (f->pointer + 38) & 0x3f;
    const uint8_t idx40 = (f->pointer + 40) & 0x3f;
    const uint8_t idx42 = (f->pointer + 42) & 0x3f;
    const uint8_t idx44 = (f->pointer + 44) & 0x3f;
    const uint8_t idx46 = (f->pointer + 46) & 0x3f;
    const uint8_t idx48 = (f->pointer + 48) & 0x3f;
    const uint8_t idx50 = (f->pointer + 50) & 0x3f;
    const uint8_t idx52 = (f->pointer + 52) & 0x3f;
    const uint8_t idx54 = (f->pointer + 54) & 0x3f;
    const uint8_t idx56 = (f->pointer + 56) & 0x3f;
    const uint8_t idx58 = (f->pointer + 58) & 0x3f;
    const uint8_t idx60 = (f->pointer + 60) & 0x3f;

    *i = ((((int32_t)f->bufi[idx2] + (int32_t)f->bufi[idx60]) * 1) +
          (((int32_t)f->bufi[idx4] + (int32_t)f->bufi[idx58]) * -6) +
          (((int32_t)f->bufi[idx6] + (int32_t)f->bufi[idx56]) * 16) +
          (((int32_t)f->bufi[idx8] + (int32_t)f->bufi[idx54]) * -32) +
          (((int32_t)f->bufi[idx10] + (int32_t)f->bufi[idx52]) * 60) +
          (((int32_t)f->bufi[idx12] + (int32_t)f->bufi[idx50]) * -102) +
          (((int32_t)f->bufi[idx14] + (int32_t)f->bufi[idx48]) * 164) +
          (((int32_t)f->bufi[idx16] + (int32_t)f->bufi[idx46]) * -254) +
          (((int32_t)f->bufi[idx18] + (int32_t)f->bufi[idx44]) * 381) +
          (((int32_t)f->bufi[idx20] + (int32_t)f->bufi[idx42]) * -561) +
          (((int32_t)f->bufi[idx22] + (int32_t)f->bufi[idx40]) * 818) +
          (((int32_t)f->bufi[idx24] + (int32_t)f->bufi[idx38]) * -1209) +
          (((int32_t)f->bufi[idx26] + (int32_t)f->bufi[idx36]) * 1876) +
          (((int32_t)f->bufi[idx28] + (int32_t)f->bufi[idx34]) * -3347) +
          (((int32_t)f->bufi[idx30] + (int32_t)f->bufi[idx32]) * 10387) +
          (((int32_t)f->bufi[idx31]) * 16384)) >> 15;

    *q = ((((int32_t)f->bufq[idx2] + (int32_t)f->bufq[idx60]) * 1) +
          (((int32_t)f->bufq[idx4] + (int32_t)f->bufq[idx58]) * -6) +
          (((int32_t)f->bufq[idx6] + (int32_t)f->bufq[idx56]) * 16) +
          (((int32_t)f->bufq[idx8] + (int32_t)f->bufq[idx54]) * -32) +
          (((int32_t)f->bufq[idx10] + (int32_t)f->bufq[idx52]) * 60) +
          (((int32_t)f->bufq[idx12] + (int32_t)f->bufq[idx50]) * -102) +
          (((int32_t)f->bufq[idx14] + (int32_t)f->bufq[idx48]) * 164) +
          (((int32_t)f->bufq[idx16] + (int32_t)f->bufq[idx46]) * -254) +
          (((int32_t)f->bufq[idx18] + (int32_t)f->bufq[idx44]) * 381) +
          (((int32_t)f->bufq[idx20] + (int32_t)f->bufq[idx42]) * -561) +
          (((int32_t)f->bufq[idx22] + (int32_t)f->bufq[idx40]) * 818) +
          (((int32_t)f->bufq[idx24] + (int32_t)f->bufq[idx38]) * -1209) +
          (((int32_t)f->bufq[idx26] + (int32_t)f->bufq[idx36]) * 1876) +
          (((int32_t)f->bufq[idx28] + (int32_t)f->bufq[idx34]) * -3347) +
          (((int32_t)f->bufq[idx30] + (int32_t)f->bufq[idx32]) * 10387) +
          (((int32_t)f->bufq[idx31]) * 16384)) >> 15;
}

// --- CORDIC implementation ---
int16_t thetas[17];
const int16_t half_pi = 16384;
int16_t recip_gain;

void cordic_init() {
    double k = 1.0;
    for(int idx=0; idx<=CORDIC_ITER; idx++){
        thetas[idx] = round(atan(k) * 32768.0 / PI);
        k *= 0.5;
    }
    double gain = 1.0;
    for(int idx=0; idx<CORDIC_ITER; idx++){
        double d = pow(2.0, (double)idx);
        gain *= sqrt(1 + 1.0/(d*d));
    }
    recip_gain = round(32767.0 / gain);
}

void cordic_rectangular_to_polar(int16_t i, int16_t q, uint16_t *mag, int16_t *phase){
    int32_t i32 = i, q32 = q, temp;
    if(i32<0){
        temp=i32;
        if(q32>0){i32=q32; q32=-temp; *phase=-half_pi;}
        else {i32=-q32; q32=temp; *phase=half_pi;}
    } else *phase=0;
    for(int idx=0; idx<=CORDIC_ITER; idx++){
        temp=i32;
        if(q32>=0){i32+=q32>>idx; q32-=temp>>idx; *phase-=thetas[idx];}
        else {i32-=q32>>idx; q32+=temp>>idx; *phase+=thetas[idx];}
    }
    *mag=(i32*recip_gain)>>14;
}

// --- SSB Demod implementation ---
half_band_filter ssb_filter;
uint8_t ssb_phase=0;
int16_t last_phase=0;
int32_t smoothed_sample=0;

void decode_iq(int16_t i, int16_t q, int16_t *out){
    uint16_t mag;
    int16_t phase;
    cordic_rectangular_to_polar(i,q,&mag,&phase);
    int16_t freq = last_phase - phase;
    last_phase = phase;
    if(freq>16384) freq-=32768;
    if(freq<-16384) freq+=32768;
    int16_t sample = ((int32_t)freq * SAMPLE_RATE)>>16;
    smoothed_sample = ((smoothed_sample<<3)+sample-smoothed_sample)>>3;
    *out = (int16_t)smoothed_sample;
}

void decode_audio(int16_t audio, int16_t *freq_out){
    ssb_phase = (ssb_phase+1)&3u;
    audio >>=1;
    const int16_t audio_i[4]={audio,0,(int16_t)-audio,0};
    const int16_t audio_q[4]={0,(int16_t)-audio,0,audio};
    int16_t ii=audio_i[ssb_phase], qq=audio_q[ssb_phase];
    filter(&ssb_filter,&ii,&qq);
    const int16_t sample_i[4]={(int16_t)-qq,(int16_t)-ii,qq,ii};
    const int16_t sample_q[4]={ii,(int16_t)-qq,(int16_t)-ii,qq};
    int16_t i=sample_i[ssb_phase], q=sample_q[ssb_phase];
    decode_iq(i,q,freq_out);
}