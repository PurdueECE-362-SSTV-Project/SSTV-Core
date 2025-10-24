#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/xosc.h"
#include "pico/multicore.h"
#include "../lib/kissfft-master/kiss_fft.h"
#include "../lib/kissfft-master/kiss_fftr.h"

typedef struct {
    Mode decMode;  // Decode mode
    ColorFormat color; // RGB or YCrCb
    int row;       // Number of rows
    int col;       // Nymber of columns
    int tranTime;  // Total transfer time (s)
    float lineTime;  // Line scan time (ms)
    float lineS;   // Line sync (ms)
    float colorS;  // Color sync (ms) ONLY for YCrCb
    int format;    // ONLY Robot type modes (1 = 4:02:00, 2 = 4:02:02 )
} sstv_mode_t;

typedef enum{ 
    GBR, // RGB
    YCRCB,  // YCrCb
} ColorFormat;

typedef enum { 
    // Binary input stream of respective VIS code
    ROBOT_36  = 0001000, 
    ROBOT_72  = 0001100, 
    MARTIN_2  = 0101000, 
    SCOTTIE_2 = 0111000, 
    PD_50     = 1011101,
    PD_90     = 1100011  
 } Mode;


 typedef enum {
    IDLE, SYNC1, SYNC_HOLD, SYNC2, START_BIT, CODE, PARITY_BIT, STOP_BIT, TRANSMISSION_ERROR
 } Header_State; 

  typedef struct {
    Header_State curState;
    uint16_t freq;      // Frequency (upper bound if applicable)
    uint16_t freq_lb;   // Frequency lower bound
    uint16_t diff_time; // Difference in time in ms
    uint16_t code;      // VIS Code
 } Decoder_FSM_Val;

// Global Constants
Decoder_FSM_Val h_idle   = {.curState = IDLE,       .freq = NULL, .freq_lb = NULL, .diff_time = NULL};
Decoder_FSM_Val h_sync1  = {.curState = SYNC1,      .freq = 1900, .freq_lb = NULL, .diff_time = 300};
Decoder_FSM_Val h_hold   = {.curState = SYNC_HOLD,  .freq = 1200, .freq_lb = NULL, .diff_time = 10};
Decoder_FSM_Val h_sync2  = {.curState = SYNC2,      .freq = 1900, .freq_lb = NULL, .diff_time = 300};
Decoder_FSM_Val h_start  = {.curState = START_BIT,  .freq = 1200, .freq_lb = NULL, .diff_time = 30};
Decoder_FSM_Val h_code   = {.curState = CODE,       .freq = 1300, .freq_lb = 1100, .diff_time = 240};
Decoder_FSM_Val h_parity = {.curState = PARITY_BIT, .freq = 1300, .freq_lb = 1100, .diff_time = 30};
Decoder_FSM_Val h_stop   = {.curState = STOP_BIT,   .freq = 1200, .freq_lb = NULL, .diff_time = 30};
// Decoder_FSM_Val h_idle = {.curState = TRANSMISSION_ERROR, .freq = 2300, .freq_lb = NULL, .diff_time = 300};

sstv_mode_t Robot36  =  {.decMode = ROBOT_36, .color = YCRCB, .row = 320, .col = 240, .tranTime = 36, .lineTime = 135,     .lineS = 10.5,  .colorS = 4.5, .format = 1};
sstv_mode_t Robot72  =  {.decMode = ROBOT_72, .color = YCRCB, .row = 320, .col = 240, .tranTime = 72, .lineTime = 276,     .lineS = 12.0,  .colorS = 6.0, .format = 2};
sstv_mode_t Martin2  =  {.decMode = MARTIN_2, .color = GBR,   .row = 256, .col = 160, .tranTime = 58, .lineTime = 219.648, .lineS = 4.862, .colorS = 0.0, .format = NULL};
sstv_mode_t Scottie2 =  {.decMode = SCOTTIE_2,.color = GBR,   .row = 256, .col = 160, .tranTime = 71, .lineTime = 264.192, .lineS = 9.0,   .colorS = 0.0, .format = NULL};
sstv_mode_t PD50   =    {.decMode = PD_50,    .color = YCRCB, .row = 320, .col = 240, .tranTime = 50, .lineTime = 183.04,  .lineS = 20.0,  .colorS = 0.0, .format = NULL};
sstv_mode_t PD90   =    {.decMode = PD_90,    .color = YCRCB, .row = 320, .col = 240, .tranTime = 90, .lineTime = 340.48,  .lineS = 20.0,  .colorS = 0.0, .format = NULL};
