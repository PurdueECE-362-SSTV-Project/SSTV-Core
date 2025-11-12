#ifndef SSTV_DECODER_H
#define SSTV_DECODER_H

#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"

// Include FFT Global Variables
#include "FFT/fft_functions.h"

// Funtion Definations  //
// Init
sstv_mode_t initializeMode(int code);

int header_fsm(Decoder_FSM_Val in);
void decoder();

// Time
void time_incrementer(bool inc_time);
void update_time();

// Expected Val
void update_exp() 

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
    IDLE, TRANS_ERROR,
    SYNC1, SYNC_HOLD, SYNC2, 
    START_BIT, CODE, PARITY_BIT, STOP_BIT,
    DECODE_MODE
 } Header_State; 

  typedef struct {
    Header_State curState;
    uint16_t freq;      // Frequency (upper bound if applicable)
    uint16_t freq_lb;   // Frequency lower bound
    uint16_t diff_time; // Difference in time in ms
 } Decoder_FSM_Val;

 // Time Keeping
//#define TIMER1_ALARM0 ((timer1_hw->timerawh) << 32 | (timer1_hw->timerawl))
uint16_t diffrential_time = 0;

// Threshold values
#define FREQ_TH 50 // +/- Hz
#define	TIME_DIFF_MS_TH 5 // +/- ms

#define THRESHOLD(ref_val, target, thr)                     ((ref_val) <= ((target) + (thr)) && (ref_val) >= ((target) - (thr)) )
#define OUT_OF_BOUND(ref_val, target, thr)                  ((ref_val) > ((target) + (thr)))
#define BOUND(ref_val, upper_limit, lower_limit, thr)       ((ref_val) <= ((upper_limit) + (thr)) && (ref_val) >= ((lower_limit) - (thr)))

// Threshold Check
#define THRESH_FREQ(ref_val, target)    THRESHOLD(ref_val, target, FREQ_TH)
#define THRESH_TIME(target)             THRESHOLD(diffrential_time, target, TIME_DIFF_MS_TH)

// Upper bound and lower bound
#define BOUND_FREQ(ref_val, upper, lower)     BOUND(ref_val, upper, lower, FREQ_TH)
#define BOUND_TIME(ref_val, upper, lower)     BOUND(ref_val, upper, lower, TIME_DIFF_MS_TH)

// Out of Bound Logic
#define TIME_OOB(target)    OUT_OF_BOUND(diffrential_time, target, TIME_DIFF_MS_TH)

#endif