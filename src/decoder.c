#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/xosc.h"
#include "pico/multicore.h"
#include "../lib/kissfft-master/kiss_fft.h"
#include "../lib/kissfft-master/kiss_fftr.h"

#include "decoder.h"

void header() {
    Header_State state = IDLE;
    Header_State nextState;
    Decoder_FSM_Val in;
    uint16_t VIS_Code;
    uint bit;
    uint8_t codeSum; // Used to compute parity

    switch (state) {
        case (SYNC1):
            if (in.freq == h_sync1.freq && in.diff_time == h_sync1.diff_time) nextState = SYNC_HOLD;
            else nextState = state;
            break;

        case (SYNC_HOLD):
            if (in.freq == h_hold.freq && in.diff_time == h_hold.diff_time) nextState = SYNC2;
            else if (in.freq != h_hold.freq || in.diff_time > h_hold.diff_time) nextState = TRANSMISSION_ERROR;
            break;

        case (SYNC2):
            if (in.freq == h_sync2.freq && in.diff_time == h_sync2.diff_time) nextState = START_BIT;
            else if (in.freq != h_sync2.freq || in.diff_time > h_sync2.diff_time) nextState = TRANSMISSION_ERROR;
            break;

        case (START_BIT):
            if (in.freq == h_start.freq && in.diff_time == h_start.diff_time) nextState = CODE;
            else if (in.freq != h_start.freq || in.diff_time > h_start.diff_time) nextState = TRANSMISSION_ERROR;
            break;

        case (CODE):
            if (in.freq <= h_code.freq && in.freq >= h_code.freq_lb && in.diff_time == h_code.diff_time) nextState = PARITY_BIT;
            else if (in.freq != h_code.freq || in.diff_time > h_code.diff_time) nextState = TRANSMISSION_ERROR;
            
            if (in.freq == 1300) bit = 0; if (in.freq == 1100) bit = 1; // Determine input binary val
            VIS_Code = (VIS_Code << 1u) + bit; // Add value to code
            break;

        case(PARITY_BIT):
            if (in.freq <= h_parity.freq && in.freq >= h_parity.freq_lb  && in.diff_time == h_parity.diff_time) nextState = STOP_BIT;
            else if (in.freq != h_parity.freq || in.diff_time > h_parity.diff_time) nextState = TRANSMISSION_ERROR;

            uint16_t temp = VIS_Code;
            if (in.freq == 1300) bit = 0; if (in.freq == 1100) bit = 1; // Determine parity bit
            for (int i = 0; i < 7; i++) {
                codeSum += (temp % 10); // Add LSB to sum
                temp /= 10;             // Shift code right 1 bit
            }
            if (codeSum + bit % 2) nextState = TRANSMISSION_ERROR; // Parity mismatch
            break;

        case(STOP_BIT):
            if (in.freq == h_stop.freq && in.diff_time == h_stop.diff_time) nextState = SYNC1;
            else if (in.freq != h_stop.freq || in.diff_time > h_stop.diff_time) nextState = TRANSMISSION_ERROR;
            break;

        case (TRANSMISSION_ERROR):
            nextState = SYNC1;
            printf("Transmission Error");
            break;

        default:
            nextState = SYNC1;
            break;
    }
    return;
}

void decoder() {
    return; 
}