#ifndef SSTV_DECODER_H
#define SSTV_DECODER_H

#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"

// Include FFT Global Variables
#include "fft_functions.h"

 // Time Keeping
extern volatile float diffrential_time;

typedef enum{
    GBR, // RGB
    YCRCB,  // YCrCb
} ColorFormat;

typedef enum {
    // Binary input stream of respective VIS code
    ROBOT_36  = 0b00010001,
    ROBOT_72  = 0b00011000,
    MARTIN_2  = 0b01010000,
    MARTIN_1  = 0b00110101,
    SCOTTIE_2 = 0b01110001,
    PD_50     = 0b10111011,
    PD_90     = 0b11000110,
    NULL_type = 0b0
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

  typedef enum {
    C_IDLE, COLOUR, SYNC
 } Colour_State;

  typedef struct {
    Header_State State;
    uint16_t freq;      // Frequency (upper bound if applicable)
    uint16_t freq_lb;   // Frequency lower bound
    uint16_t diff_time; // Difference in time in ms
 } Decoder_FSM_Val;

   typedef struct {
    Colour_State State;
    uint16_t freq;      // Frequency (upper bound if applicable)
    uint16_t freq_lb;   // Frequency lower bound
 } Colour_Decoder_Val;

// Funtion Definations  //
// Init
sstv_mode_t initVISMode(uint8_t);

int header_fsm(uint16_t);
void decoder(int16_t);
void colour_decoder(uint16_t);

// Time
void time_incrementer(bool);
void update_time();

// Expected Val
void update_decoder_exp();
void update_colour_decoder_exp();

// Print a binary number
void print_header_fsm(uint16_t);

// Next state logic
void nextState_thresh (uint16_t, Decoder_FSM_Val, Decoder_FSM_Val);

void nextState_bound (uint16_t, Decoder_FSM_Val, Decoder_FSM_Val);
void nextState_intoBound (uint16_t, Decoder_FSM_Val, Decoder_FSM_Val);

void nextState_intoBoundRange (uint16_t, Decoder_FSM_Val, Decoder_FSM_Val);
void nextState_boundRange (uint16_t, Decoder_FSM_Val, Decoder_FSM_Val);

// Colour Decoding Logi
void nextColourState_toBound (uint16_t, Colour_Decoder_Val, Colour_Decoder_Val);
void nextColourState_toThresh (uint16_t, Colour_Decoder_Val, Colour_Decoder_Val);

// Threshold values
#define FREQ_TH 75 // +/- Hz
#define TIME_DIFF_MS_TH 5 // +/- ms

#define THRESHOLD(ref_val, target, thr)                         ((ref_val <= (target + thr)) && (ref_val >= (target - thr)))
#define OUT_OF_BOUND(ref_val, target, thr)                      (ref_val > (target + thr))
#define BOUND_RANGE(ref_val, upper_limit, lower_limit, thr)     ((ref_val <= (upper_limit + thr)) && (ref_val >= (lower_limit - thr)))

// Threshold Check
#define THRESH_FREQ(ref_val, target)    THRESHOLD(ref_val, target, FREQ_TH)
#define THRESH_TIME(target)             THRESHOLD(diffrential_time, target, TIME_DIFF_MS_TH)

// Upper bound and lower bound
#define BOUND_FREQ(ref_val, upper, lower)     (THRESH_FREQ(ref_val, upper) || THRESH_FREQ(ref_val, lower))
#define BOUND_TIME(ref_val, upper, lower)     (THRESH_TIME(ref_val, upper) || THRESH_TIME(ref_val, lower))

// Upper bound and lower bound for a
#define BOUND_RANGE_FREQ(ref_val, upper, lower)     BOUND_RANGE(ref_val, upper, lower, FREQ_TH)
#define BOUND_RANGE_TIME(ref_val, upper, lower)     BOUND_RANGE(ref_val, upper, lower, TIME_DIFF_MS_TH)

// Out of Bound Logic
#define TIME_OOB(target)    OUT_OF_BOUND(diffrential_time, target, TIME_DIFF_MS_TH)

#endif