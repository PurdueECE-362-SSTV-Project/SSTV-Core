#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/xosc.h"
#include "pico/multicore.h"
#include "../lib/kissfft-master/kiss_fft.h"
#include "../lib/kissfft-master/kiss_fftr.h"

#include "decoder.h"

// ASYNC FSM Vars
const Decoder_FSM_Val h_idle      = {.curState  = IDLE,        .freq = 0,      .freq_lb = NULL,    .diff_time = 0};
const Decoder_FSM_Val h_sync1     = {.curState  = SYNC1,       .freq = 1900,   .freq_lb = NULL,    .diff_time = 300};
const Decoder_FSM_Val h_hold      = {.curState  = SYNC_HOLD,   .freq = 1200,   .freq_lb = NULL,    .diff_time = 10};
const Decoder_FSM_Val h_sync2     = {.curState  = SYNC2,       .freq = 1900,   .freq_lb = NULL,    .diff_time = 300};
const Decoder_FSM_Val h_start     = {.curState  = START_BIT,   .freq = 1200,   .freq_lb = NULL,    .diff_time = 30};
const Decoder_FSM_Val h_code      = {.curState  = CODE,        .freq = 1300,   .freq_lb = 1100,    .diff_time = 240};
const Decoder_FSM_Val h_parity    = {.curState  = PARITY_BIT,  .freq = 1300,   .freq_lb = 1100,    .diff_time = 30};
const Decoder_FSM_Val h_stop      = {.curState  = STOP_BIT,    .freq = 1200,   .freq_lb = NULL,    .diff_time = 30};
const Decoder_FSM_Val h_tError    = {.curState  = TRANS_ERROR, .freq = 2300,   .freq_lb = NULL,    .diff_time = 300};     // Different Kind of IDLE
const Decoder_FSM_Val h_decode    = {.curState  = DECODE_MODE, .freq = 2300,   .freq_lb = 1100,    .diff_time = 300};     // Goes into Decoding mode

// SSTV MODE Vars
const sstv_mode_t Robot36     =  {.decMode = ROBOT_36,    .color = YCRCB, .row = 320,     .col = 240,     .tranTime = 36,     .lineTime = 135,     .lineS = 10.5,  .colorS = 4.5, .format = 1};
const sstv_mode_t Robot72     =  {.decMode = ROBOT_72,    .color = YCRCB, .row = 320,     .col = 240,     .tranTime = 72,     .lineTime = 276,     .lineS = 12.0,  .colorS = 6.0, .format = 2};
const sstv_mode_t Martin2     =  {.decMode = MARTIN_2,    .color = GBR,   .row = 256,     .col = 160,     .tranTime = 58,     .lineTime = 219.648, .lineS = 4.862, .colorS = 0.0, .format = 0};
const sstv_mode_t Scottie2    =  {.decMode = SCOTTIE_2,   .color = GBR,   .row = 256,     .col = 160,     .tranTime = 71,     .lineTime = 264.192, .lineS = 9.0,   .colorS = 0.0, .format = 0};
const sstv_mode_t PD50        =  {.decMode = PD_50,       .color = YCRCB, .row = 320,     .col = 240,     .tranTime = 50,     .lineTime = 183.04,  .lineS = 20.0,  .colorS = 0.0, .format = 0};
const sstv_mode_t PD90        =  {.decMode = PD_90,       .color = YCRCB, .row = 320,     .col = 240,     .tranTime = 90,     .lineTime = 340.48,  .lineS = 20.0,  .colorS = 0.0, .format = 0};
const sstv_mode_t NULL_Mode   =  {.decMode = NULL,        .color = NULL,  .row = NULL,    .col = NULL,    .tranTime = NULL,   .lineTime = 0,       .lineS = 0,     .colorS = 0,   .format = NULL};

// Global Variables
Header_State h_nextState   = IDLE;    // Stores current state of header
Header_State h_currState   = IDLE;    // Stores Previous state of header

uint16_t VIS_Code   = 0;            // Stores the VIS Code
uint8_t codeSum     = 0;            // Used to compute parity
uint bit            = 0;            // What bit it's currently on

// Global FSM Holder
Decoder_FSM_Val FSM_exp_curr = h_idle;        // CurrState exp Freq

int header_fsm(Decoder_FSM_Val in, uint16_t freq) {
    // Update CurrState
    h_currState = h_nextState;

    // Check if Frequency Changes (thresh)
    // If Freq change - check if time within bound
    // Else - go to IDLE
    swtich(h_currState) {
        case (SYNC1): 
                // If not within threshold - freq changed to next_state freq
                if (!THRESH_FREQ(freq, FSM_exp_curr.freq))
                    // Diff_time is within Bounds - Move to next_state
                    if (THRESH_TIME(FSM_exp_curr.diff_time)) 
                        h_nextState = SYNC_HOLD;

                    // If Time if Out of Bounds - go to IDLE (error)
                    else 
                        h_nextState = IDLE;

                // If stays in state longer than it's supposed to
                else if (TIME_OOB(FSM_exp_curr.diff_time))
                    h_nextState = IDLE;

                // If within Threshold - remain in state
                else
                    h_nextState = SYNC1;
            break;

        case (SYNC_HOLD): 
                // If not within threshold - freq changed to next_state freq
                if (!THRESH_FREQ(freq, FSM_exp_curr.freq))
                    // Diff_time is within Bounds - Move to next_state
                    if (THRESH_TIME(FSM_exp_curr.diff_time)) 
                        h_nextState = SYNC2;

                    // If Time if Out of Bounds - go to IDLE (error)
                    else 
                        h_nextState = IDLE;

                // If stays in state longer than it's supposed to
                else if (TIME_OOB(FSM_exp_curr.diff_time))
                    h_nextState = IDLE;

                // If within Threshold - remain in state
                else
                    h_nextState = SYNC_HOLD;
            break;
        
        default:
                h_nextState = IDLE;
            break;
    }

    switch (h_nextState) {
        case (IDLE): 
            if (THRESHOLD(in.freq, h_sync1.freq, FREQ_TH) && THRESHOLD(in.diff_time, h_sync1.diff_time, TIME_DIFF_MS_TH)) h_nextState = SYNC_HOLD;
            else h_nextState = h_nextState;
            break;

        case (SYNC1): 
            if (THRESHOLD(in.freq, h_sync1.freq, FREQ_TH) && THRESHOLD(in.diff_time, h_sync1.diff_time, TIME_DIFF_MS_TH)) h_nextState = SYNC_HOLD;
            else h_nextState = h_nextState;
            break;

        case (SYNC_HOLD):
            if (THRESHOLD(in.freq, h_hold.freq, FREQ_TH) && THRESHOLD(in.diff_time, h_hold.diff_time, TIME_DIFF_MS_TH)) h_nextState = SYNC2;
            else if (!THRESHOLD(in.freq, h_hold.freq, FREQ_TH) || in.diff_time > (h_hold.diff_time + TIME_DIFF_MS_TH)) h_nextState = TRANS_ERROR;
            else h_nextState = h_nextState;
            break;

        case (SYNC2):
            if (THRESHOLD(in.freq, h_sync2.freq, FREQ_TH) && THRESHOLD(in.diff_time, h_sync2.diff_time, TIME_DIFF_MS_TH)) h_nextState = START_BIT;
            else if (THRESHOLD(in.freq, h_hold.freq, FREQ_TH) && THRESHOLD(in.diff_time, h_hold.diff_time, TIME_DIFF_MS_TH)) h_nextState = SYNC2; // Overlapping case
            else if (!THRESHOLD(in.freq, h_sync2.freq, FREQ_TH) || in.diff_time > (h_sync2.diff_time + TIME_DIFF_MS_TH)) h_nextState = TRANS_ERROR;
            else h_nextState = h_nextState;
            break;

        case (START_BIT):
            if (THRESHOLD(in.freq, h_start.freq, FREQ_TH) && THRESHOLD(in.diff_time, h_start.diff_time, TIME_DIFF_MS_TH)) h_nextState = CODE;
            else if (!THRESHOLD(in.freq, h_start.freq, FREQ_TH) || in.diff_time > (h_start.diff_time + TIME_DIFF_MS_TH)) h_nextState = TRANS_ERROR;
            else h_nextState = h_nextState;
            break;

        case (CODE):
            if (THRESHOLD(in.freq, h_code.freq, FREQ_TH) && THRESHOLD(in.diff_time, h_code.diff_time, TIME_DIFF_MS_TH)) h_nextState = PARITY_BIT;
            else if (!THRESHOLD(in.freq, h_code.freq, FREQ_TH) || in.diff_time > (h_code.diff_time + TIME_DIFF_MS_TH)) h_nextState = TRANS_ERROR;
            else h_nextState = h_nextState;
            
            if (THRESHOLD(in.freq, 1300, FREQ_TH)) bit = 0; 
            else if (THRESHOLD(in.freq, 1100, FREQ_TH)) bit = 1; // Determine input binary val
            else {
                bit = 0;
                printf("Code bit in unknown state");
            }
            VIS_Code = (VIS_Code << 1u) + bit; // Add value to code
            break;

        case(PARITY_BIT):
            if (THRESHOLD(in.freq, h_parity.freq, FREQ_TH) && THRESHOLD(in.diff_time, h_parity.diff_time, TIME_DIFF_MS_TH)) h_nextState = STOP_BIT;
            else if (!THRESHOLD(in.freq, h_parity.freq, FREQ_TH) || in.diff_time > (h_parity.diff_time + TIME_DIFF_MS_TH)) h_nextState = TRANS_ERROR;
            else h_nextState = h_nextState;

            uint16_t temp = VIS_Code;
            if (THRESHOLD(in.freq, 1300, FREQ_TH)) bit = 0;
            else if (THRESHOLD(in.freq, 1100, FREQ_TH)) bit = 1; // Determine parity bit
            else {
                bit = 0;
                printf("Parity bit in unknown state");
            }
            for (int i = 0; i < 7; i++) {
                codeSum += (temp & 1);  // Get LSB
                temp >>= 1;             // Shift right 1 bit
            }
            if ((codeSum + bit) % 2) h_nextState = TRANS_ERROR; // Detect parity mismatch
            break;

        case(STOP_BIT):
            if (THRESHOLD(in.freq, h_stop.freq, FREQ_TH) && THRESHOLD(in.diff_time, h_stop.diff_time, TIME_DIFF_MS_TH)) return VIS_Code; // If transmission is successful, return the received code
            else if (!THRESHOLD(in.freq, h_stop.freq, FREQ_TH) || in.diff_time > (h_sync2.diff_time + TIME_DIFF_MS_TH)) h_nextState = TRANS_ERROR;
            else h_nextState = h_nextState;
            break;

        case (TRANS_ERROR):
            h_nextState = SYNC1;
            printf("\nTransmission Error");
            break;

        default:
            h_nextState = SYNC1;
            break;
    }

    update_time();
    update_exp();

    return 0;
}

void update_time() {
    // if same state - inc time     (and it's not in idle)
    if ((h_nextState == h_currState) && (h_nextState != IDLE)) time_incrementer(1);
    // Else reset time increment
    else                                                        time_incrementer(0);
}

void update_exp() {
    switch (h_nextState) {
        case IDLE: 
            FSM_exp_curr = h_idle;
            break;

        case SYNC1: 
            FSM_exp_curr = h_sync1;
            break;

        case SYNC_HOLD: 
            FSM_exp_curr = h_hold;
            break;

        case SYNC2: 
            FSM_exp_curr = h_sync2;
            break;

        case START_BIT: 
            FSM_exp_curr = h_start;
            break;

        case CODE: 
            FSM_exp_curr = h_code;
            break;

        case PARITY_BIT: 
            FSM_exp_curr = h_parity;
            break;

        case STOP_BIT: 
            FSM_exp_curr = h_stop;
            break;

        case DECODE_MODE: 
            FSM_exp_curr = h_decode;
            break;
        
        default: 
            FSM_exp_curr = h_idle;
            break;
    }
}

void time_incrementer(bool inc_time) {
    if (inc_time) diffrential_time += 1.0 / SFREQ;      // Increase by sampling period
    else diffrential_time = 0;                          // if 0 then reset

    return;
}

sstv_mode_t initializeMode(int code) {
    if (code == ROBOT_36) return Robot36;
    else if (code == ROBOT_72) return Robot72;
    else if (code == MARTIN_2) return Martin2;
    else if (code == SCOTTIE_2) return Scottie2;
    else if (code == PD_50) return PD50;
    else if (code == PD_90) return PD90;
    else return NULL_Mode;
}

void decoder() {
    Decoder_FSM_Val input;

    int freq = 0;
    header_fsm(input, freq);

    // If Decode Mode
    if (h_nextState == DECODE_MODE) {
        sstv_mode_t mode = initializeMode(VIS_Code);

        // If VIS Code doesn't match
        if (mode.decMode == NULL) printf("\nReceived encoding type is not recognized");
    }
}