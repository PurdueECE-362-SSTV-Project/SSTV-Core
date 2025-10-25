#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/xosc.h"
#include "pico/multicore.h"
#include "../lib/kissfft-master/kiss_fft.h"
#include "../lib/kissfft-master/kiss_fftr.h"

#include "decoder.h"

// Global Constants
Decoder_FSM_Val h_idle   = {.curState = IDLE,       .freq = 0,    .freq_lb = 0, .diff_time = 0};
Decoder_FSM_Val h_sync1  = {.curState = SYNC1,      .freq = 1900, .freq_lb = 0, .diff_time = 300};
Decoder_FSM_Val h_hold   = {.curState = SYNC_HOLD,  .freq = 1200, .freq_lb = 0, .diff_time = 10};
Decoder_FSM_Val h_sync2  = {.curState = SYNC2,      .freq = 1900, .freq_lb = 0, .diff_time = 300};
Decoder_FSM_Val h_start  = {.curState = START_BIT,  .freq = 1200, .freq_lb = 0, .diff_time = 30};
Decoder_FSM_Val h_code   = {.curState = CODE,       .freq = 1300, .freq_lb = 1100, .diff_time = 240};
Decoder_FSM_Val h_parity = {.curState = PARITY_BIT, .freq = 1300, .freq_lb = 1100, .diff_time = 30};
Decoder_FSM_Val h_stop   = {.curState = STOP_BIT,   .freq = 1200, .freq_lb = 0, .diff_time = 30};
// Decoder_FSM_Val h_idle = {.curState = TRANSMISSION_ERROR, .freq = 2300, .freq_lb = NULL, .diff_time = 300};

sstv_mode_t Robot36  =  {.decMode = ROBOT_36, .color = YCRCB, .row = 320, .col = 240, .tranTime = 36, .lineTime = 135,     .lineS = 10.5,  .colorS = 4.5, .format = 1};
sstv_mode_t Robot72  =  {.decMode = ROBOT_72, .color = YCRCB, .row = 320, .col = 240, .tranTime = 72, .lineTime = 276,     .lineS = 12.0,  .colorS = 6.0, .format = 2};
sstv_mode_t Martin2  =  {.decMode = MARTIN_2, .color = GBR,   .row = 256, .col = 160, .tranTime = 58, .lineTime = 219.648, .lineS = 4.862, .colorS = 0.0, .format = 0};
sstv_mode_t Scottie2 =  {.decMode = SCOTTIE_2,.color = GBR,   .row = 256, .col = 160, .tranTime = 71, .lineTime = 264.192, .lineS = 9.0,   .colorS = 0.0, .format = 0};
sstv_mode_t PD50   =    {.decMode = PD_50,    .color = YCRCB, .row = 320, .col = 240, .tranTime = 50, .lineTime = 183.04,  .lineS = 20.0,  .colorS = 0.0, .format = 0};
sstv_mode_t PD90   =    {.decMode = PD_90,    .color = YCRCB, .row = 320, .col = 240, .tranTime = 90, .lineTime = 340.48,  .lineS = 20.0,  .colorS = 0.0, .format = 0};
sstv_mode_t NULL_Mode = {.decMode = 0,     .color = 0,  .row = 0, .col = 0, .tranTime = 0, .lineTime = 0, .lineS = 0, .colorS = 0, .format = 0};


int header(Decoder_FSM_Val in) {
    Header_State state, nextState;
    uint16_t VIS_Code; // Stores the VIS Code
    uint bit;
    uint8_t codeSum; // Used to compute parity

    switch (state) {
        case (SYNC1): 
            if (THRESHOLD(in.freq, h_sync1.freq, FREQ_TH) && THRESHOLD(in.diff_time, h_sync1.diff_time, TIME_DIFF_MS_TH)) nextState = SYNC_HOLD;
            else nextState = state;
            break;

        case (SYNC_HOLD):
            if (THRESHOLD(in.freq, h_hold.freq, FREQ_TH) && THRESHOLD(in.diff_time, h_hold.diff_time, TIME_DIFF_MS_TH)) nextState = SYNC2;
            else if (!THRESHOLD(in.freq, h_hold.freq, FREQ_TH) || in.diff_time > (h_hold.diff_time + TIME_DIFF_MS_TH)) nextState = TRANSMISSION_ERROR;
            else nextState = state;
            break;

        case (SYNC2):
            if (THRESHOLD(in.freq, h_sync2.freq, FREQ_TH) && THRESHOLD(in.diff_time, h_sync2.diff_time, TIME_DIFF_MS_TH)) nextState = START_BIT;
            else if (THRESHOLD(in.freq, h_hold.freq, FREQ_TH) && THRESHOLD(in.diff_time, h_hold.diff_time, TIME_DIFF_MS_TH)) nextState = SYNC2; // Overlapping case
            else if (!THRESHOLD(in.freq, h_sync2.freq, FREQ_TH) || in.diff_time > (h_sync2.diff_time + TIME_DIFF_MS_TH)) nextState = TRANSMISSION_ERROR;
            else nextState = state;
            break;

        case (START_BIT):
            if (THRESHOLD(in.freq, h_start.freq, FREQ_TH) && THRESHOLD(in.diff_time, h_start.diff_time, TIME_DIFF_MS_TH)) nextState = CODE;
            else if (!THRESHOLD(in.freq, h_start.freq, FREQ_TH) || in.diff_time > (h_start.diff_time + TIME_DIFF_MS_TH)) nextState = TRANSMISSION_ERROR;
            else nextState = state;
            break;

        case (CODE):
            if (THRESHOLD(in.freq, h_code.freq, FREQ_TH) && THRESHOLD(in.diff_time, h_code.diff_time, TIME_DIFF_MS_TH)) nextState = PARITY_BIT;
            else if (!THRESHOLD(in.freq, h_code.freq, FREQ_TH) || in.diff_time > (h_code.diff_time + TIME_DIFF_MS_TH)) nextState = TRANSMISSION_ERROR;
            else nextState = state;
            
            if (THRESHOLD(in.freq, 1300, FREQ_TH)) bit = 0; 
            else if (THRESHOLD(in.freq, 1100, FREQ_TH)) bit = 1; // Determine input binary val
            else {
                bit = 0;
                printf("Code bit in unknown state");
            }
            VIS_Code = (VIS_Code << 1u) + bit; // Add value to code
            break;

        case(PARITY_BIT):
            if (THRESHOLD(in.freq, h_parity.freq, FREQ_TH) && THRESHOLD(in.diff_time, h_parity.diff_time, TIME_DIFF_MS_TH)) nextState = STOP_BIT;
            else if (!THRESHOLD(in.freq, h_parity.freq, FREQ_TH) || in.diff_time > (h_parity.diff_time + TIME_DIFF_MS_TH)) nextState = TRANSMISSION_ERROR;
            else nextState = state;

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
            if ((codeSum + bit) % 2) nextState = TRANSMISSION_ERROR; // Detect parity mismatch
            break;

        case(STOP_BIT):
            if (THRESHOLD(in.freq, h_stop.freq, FREQ_TH) && THRESHOLD(in.diff_time, h_stop.diff_time, TIME_DIFF_MS_TH)) return VIS_Code; // If transmission is successful, return the received code
            else if (!THRESHOLD(in.freq, h_stop.freq, FREQ_TH) || in.diff_time > (h_sync2.diff_time + TIME_DIFF_MS_TH)) nextState = TRANSMISSION_ERROR;
            else nextState = state;
            break;

        case (TRANSMISSION_ERROR):
            nextState = SYNC1;
            printf("\nTransmission Error");
            break;

        default:
            nextState = SYNC1;
            break;
    }
    return 0;
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
    int VIS_Code = header(input);
    sstv_mode_t mode = initializeMode(VIS_Code);
    if (mode.decMode == 0) printf("\nReceived encoding type is not recognized");
    return; 
}