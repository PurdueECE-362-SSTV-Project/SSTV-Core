#ifndef SSTV_DECODER_H
#define SSTV_DECODER_H

#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/xosc.h"
#include "pico/multicore.h"
#include "../lib/kissfft-master/kiss_fft.h"
#include "../lib/kissfft-master/kiss_fftr.h"

typedef enum{ 
    GBR, // RGB
    YCRCB,  // YCrCb
} ColorFormat;

typedef enum { 
    // Binary input stream of respective VIS code
    ROBOT_36  = 0b0001000, 
    ROBOT_72  = 0b0001100, 
    MARTIN_2  = 0b0101000, 
    SCOTTIE_2 = 0b0111000, 
    PD_50     = 0b1011101,
    PD_90     = 0b1100011  
 } SSTV_Mode;

 
typedef struct {
    SSTV_Mode decMode;  // Decode mode
    ColorFormat color; // RGB or YCrCb
    int row;       // Number of rows
    int col;       // Nymber of columns
    int tranTime;  // Total transfer time (s)
    float lineTime;  // Line scan time (ms)
    float lineS;   // Line sync (ms)
    float colorS;  // Color sync (ms) ONLY for YCrCb
    int format;    // ONLY Robot type modes (1 = 4:02:00, 2 = 4:02:02 )
} sstv_mode_t;

 typedef enum {
    IDLE, SYNC1, SYNC_HOLD, SYNC2, START_BIT, CODE, PARITY_BIT, STOP_BIT, TRANSMISSION_ERROR
 } Header_State; 

  typedef struct {
    Header_State curState;
    uint16_t freq;      // Frequency (upper bound if applicable)
    uint16_t freq_lb;   // Frequency lower bound
    uint16_t diff_time; // Difference in time in ms
 } Decoder_FSM_Val;


// Threshold values
#define FREQ_TH 50 // +/- Hz
#define	TIME_DIFF_MS_TH 5 // +/- ms

#define THRESHOLD(val, target, thr) ((val) >= ((target) - (thr)) && (val) <= ((target) + (thr)))

#endif