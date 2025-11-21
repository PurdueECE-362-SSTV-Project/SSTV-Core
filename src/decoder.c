#include <stdio.h>

// Pico Libraries
#include "pico/stdlib.h"

// Custom
#include "decoder.h"
#include "Ili9341.h"

// ASYNC FSM Vars
static const Decoder_FSM_Val h_idle      = {.State  = IDLE,        .freq = 0,      .freq_lb = 0,    .diff_time = 0};
static const Decoder_FSM_Val h_sync1     = {.State  = SYNC1,       .freq = 1900,   .freq_lb = 0,    .diff_time = 300};
static const Decoder_FSM_Val h_hold      = {.State  = SYNC_HOLD,   .freq = 1200,   .freq_lb = 0,    .diff_time = 10};
static const Decoder_FSM_Val h_sync2     = {.State  = SYNC2,       .freq = 1900,   .freq_lb = 0,    .diff_time = 300};
static const Decoder_FSM_Val h_start     = {.State  = START_BIT,   .freq = 1200,   .freq_lb = 0,    .diff_time = 30};
static const Decoder_FSM_Val h_code      = {.State  = CODE,        .freq = 1300,   .freq_lb = 1100,    .diff_time = 240};
static const Decoder_FSM_Val h_stop      = {.State  = STOP_BIT,    .freq = 1200,   .freq_lb = 0,    .diff_time = 30};
static const Decoder_FSM_Val h_decode    = {.State  = DECODE_MODE, .freq = 2300,   .freq_lb = 1200,    .diff_time = 300};

// Colour Vars
static const Colour_Decode_Val c_line  = {.freq = 2300,    .freq_lb = 1500};
static const Colour_Decode_Val c_sync  = {.freq = 1500,    .freq_lb = 1200};

// SSTV MODE Vars
static const sstv_mode_t Robot36     =  {.decMode = ROBOT_36,    .color = YCRCB, .row = 320,     .col = 240,     .tranTime = 36,     .lineTime = 135,     .lineS = 10.5,  .colorS = 4.5, .format = 1};
static const sstv_mode_t Robot72     =  {.decMode = ROBOT_72,    .color = YCRCB, .row = 320,     .col = 240,     .tranTime = 72,     .lineTime = 276,     .lineS = 12.0,  .colorS = 6.0, .format = 2};
static const sstv_mode_t Martin1     =  {.decMode = MARTIN_1,    .color = GBR,   .row = 320,     .col = 256,     .tranTime = 114,    .lineTime = 439.296, .lineS = 4.862, .colorS = 0.0, .format = 0};
static const sstv_mode_t Martin2     =  {.decMode = MARTIN_2,    .color = GBR,   .row = 256,     .col = 160,     .tranTime = 58,     .lineTime = 219.648, .lineS = 4.862, .colorS = 0.0, .format = 0};
static const sstv_mode_t Scottie2    =  {.decMode = SCOTTIE_2,   .color = GBR,   .row = 256,     .col = 160,     .tranTime = 71,     .lineTime = 264.192, .lineS = 9.0,   .colorS = 0.0, .format = 0};
static const sstv_mode_t PD50        =  {.decMode = PD_50,       .color = YCRCB, .row = 320,     .col = 240,     .tranTime = 50,     .lineTime = 183.04,  .lineS = 20.0,  .colorS = 0.0, .format = 0};
static const sstv_mode_t PD90        =  {.decMode = PD_90,       .color = YCRCB, .row = 320,     .col = 240,     .tranTime = 90,     .lineTime = 340.48,  .lineS = 20.0,  .colorS = 0.0, .format = 0};
static const sstv_mode_t NULL_mode   =  {.decMode = NULL_type,   .color = GBR,   .row = 0,       .col = 0,       .tranTime = 0,      .lineTime = 0,       .lineS = 0,     .colorS = 0,   .format = 0};

#define VIS_BITS_TOTAL   8        // Number of bits in a VIS code
#define SAMPLES_PER_BIT  450       // Number of samples per bit (~30ms)  

// For initing Vars
static bool vars_init = false;

// VIS CODE Flags
static bool new_vis_code = true;
static uint8_t VIS_Code   = 0;            // Stores the VIS Code   

static uint64_t sample_count = 0;
static uint32_t VIS_CODE_upper = 0;
static uint32_t VIS_CODE_lower = 0;
static sstv_mode_t     curr_VIS_mode      = NULL_mode;

// Decoding Vars
static uint8_t curr_y = 0;
static uint8_t curr_x = 0;
static uint8_t curr_sample = 0;
static bool y_updated = 0;

// Global FSM Holder
Decoder_FSM_Val h_prevState     = h_idle;        // PrevState Freq
Decoder_FSM_Val h_currState     = h_idle;        // CurrState Freq
Decoder_FSM_Val h_nextState     = h_idle;        // NextState Freq
Decoder_FSM_Val h_nextState_exp = h_idle;        // NextState exp Freq

// History Bits
static uint32_t in_bounds_history  = 0;
static uint64_t in_bounds_time_history = 0;

// Store VIS Mode and Colour Decoding 
#define GET_MODE_WIDTH      curr_VIS_mode.row
#define GET_MODE_HEIGHT     curr_VIS_mode.col
#define GET_LINE_TIME       curr_VIS_mode.lineTime

// Printing Logic (AUX)
static uint16_t print_count = 0;
static uint16_t print_colour = 0;
static uint16_t print_sync = 0;

// Decoder FSM
void decoder(int16_t in_buff) {
    header_fsm(in_buff);
}

// Initializing Different Modes
sstv_mode_t initVISMode(uint8_t code) {
    if      (code == (uint8_t) ROBOT_36)  return Robot36;
    else if (code == (uint8_t) ROBOT_72)  return Robot72;
    else if (code == (uint8_t) MARTIN_1)  return Martin1;
    else if (code == (uint8_t) MARTIN_2)  return Martin2;
    else if (code == (uint8_t) SCOTTIE_2) return Scottie2;
    else if (code == (uint8_t) PD_50)     return PD50;
    else if (code == (uint8_t) PD_90)     return PD90;
    else {
            printf("\nReceived encoding type is not recognized");
            return NULL_mode;
    }
}

int header_fsm(uint16_t freq) {
    h_currState = h_nextState;

    // FSM Newstate printing Logic
    print_header_fsm(freq);

    // Check if Frequency Changes (thresh)
    // If Freq change - check if time within bound
    // Else - go to IDLE
    switch(h_currState.State) {
        case (IDLE):
                nextState_thresh(freq, h_sync1, h_idle);
            break;

        case (SYNC1):
                nextState_thresh(freq, h_hold, h_idle);
            break;

        case (SYNC_HOLD):
                nextState_thresh(freq, h_sync2, h_idle);
            break;

        case (SYNC2):
                nextState_thresh(freq, h_start, h_idle);

                // Get ready to init vars
                vars_init = false;
            break;

        case (START_BIT):
                if (!vars_init) {
                    // init all vairs for VIS code reading
                    sample_count = 0;
                    VIS_CODE_upper = 0;
                    VIS_CODE_lower = 0;
                    new_vis_code = true;

                    // Init Decoding Vars
                    curr_y = 0;
                    curr_x = 0;
                    curr_sample = 0;
                    y_updated = 0;

                    // Vars inited
                    vars_init = true;
                }

                nextState_intoBound(freq, h_code, h_idle);
            break;

        case (CODE):
            nextState_bound(freq, h_stop, h_idle);

            VIS_CODE_upper = (VIS_CODE_upper << 1) | THRESH_FREQ(freq, 1300);
            VIS_CODE_lower = (VIS_CODE_lower << 1) | THRESH_FREQ(freq, 1100);

            // New code can be written
            if (new_vis_code) {
                if (!(VIS_CODE_upper + 1)) {
                    new_vis_code = false;
                    VIS_Code = (VIS_Code << 1) | 0;
                }

                else if (!(VIS_CODE_lower + 1)) {
                    new_vis_code = false;
                    VIS_Code = (VIS_Code << 1) | 1;
                }
            }

            // When 30 ms - new sample can be written
            if (!(sample_count % SAMPLES_PER_BIT)) {
                new_vis_code = true;
                VIS_CODE_upper = VIS_CODE_lower = 0;
            }
            sample_count++;

            break;

        case (STOP_BIT):
                // Print VIS Code bits
                //print_binary(VIS_Code, 8);
                curr_VIS_mode = initVISMode(VIS_Code);

                nextState_intoBoundRange(freq, h_decode, h_idle);
            break;

        case (DECODE_MODE):
                // Colour Decoding FSM
                colour_decode(freq);

                // Move out of decode mode
                nextState_boundRange(freq, h_idle, h_idle);
            break;

        default:
                h_nextState = h_idle;
            break;
    }

    update_decoder_exp();
    h_prevState = h_currState;

    return 0;
}

// Print Header from FSM
void print_header_fsm(uint16_t freq) {
    //only print when current state differs from prev state
    if(h_prevState.State != h_currState.State) {
        printf("sample freq: %d, Current State: ", freq);
        switch (h_currState.State) {
            case IDLE:
                printf("IDLE\n");
                break;

            case SYNC1:
                printf("SYNC1\n");
                break;

            case SYNC_HOLD:
                printf("SYNC_HOLD\n");
                break;

            case SYNC2:
                printf("SYNC2\n");
                break;

            case START_BIT:
                printf("START_BIT\n");
                break;

            case CODE:
                printf("CODE\n");
                break;

            case PARITY_BIT:
                printf("PARITY_BIT\n");
                break;

            case STOP_BIT:
                printf("STOP_BIT\n");
                break;

            case DECODE_MODE:
                printf("DECODE_MODE\n");
                break;

            default:
                //printf("IDLE\n");
                break;
        }
    }
}

void update_decoder_exp() {
    switch (h_currState.State) {
        case IDLE:
            h_nextState_exp = h_sync1;

            // History reset
            in_bounds_time_history = 0;
            break;

        case SYNC1:
            h_nextState_exp = h_hold;
            break;

        case SYNC_HOLD:
            h_nextState_exp = h_sync2;
            break;

        case SYNC2:
            h_nextState_exp = h_start;
            break;

        case START_BIT:
            h_nextState_exp = h_code;
            break;

        case CODE:
            h_nextState_exp = h_stop;
            break;

        case STOP_BIT:
            h_nextState_exp = h_decode;
            break;

        case DECODE_MODE:
            h_nextState_exp = h_idle;
            break;

        default:
            h_nextState_exp = h_idle;

            // History reset
            in_bounds_time_history = 0;
            in_bounds_history = 0;
            printf("IDLE\n");
            break;
    }
}

void colour_decode(uint16_t freq) {
    if BOUND_RANGE_FREQ(freq, c_line.freq, c_line.freq_lb) {
        y_updated = 0;
        curr_sample++;           // Update x

        // Do colour Logic
        // PIX(curr_x, curr_y) = 
    }
    else if BOUND_RANGE_FREQ(freq, c_sync.freq, c_sync.freq_lb) {
        if (!y_updated) {
            curr_y++;
            curr_sample = 0;

            // Don't reupdate
            y_updated = 1;
        }
        // Sync Logic
    }
    // Else out of bound
}

void nextState_thresh (uint16_t freq, Decoder_FSM_Val move_into, Decoder_FSM_Val move_failed) {
    h_nextState = h_currState;

    // If not within threshold - freq changed to next_state freq
    if (!(THRESH_FREQ(freq, h_currState.freq))){
        in_bounds_time_history = in_bounds_time_history << 1u | 1u;
        in_bounds_history = in_bounds_history << 1 | THRESH_FREQ(freq, h_nextState_exp.freq);
        // Freq is within Threshold - Move to next_state
        if((in_bounds_time_history + 1) != 0) {
            if ((in_bounds_history + 1) == 0)
            {
                h_nextState = move_into;

                // Reset Histories
                in_bounds_time_history = 0;
                in_bounds_history = 0;
            }
        }

    // Time out of bounds
        else {
            h_nextState = move_failed;
            in_bounds_time_history = 0;
            in_bounds_history = 0;
        }
    }
}

void nextState_intoBound (uint16_t freq, Decoder_FSM_Val move_into, Decoder_FSM_Val move_failed) {
    h_nextState = h_currState;

    // If not within bound - freq changed to next_state freq
    if (!(THRESH_FREQ(freq, h_currState.freq))) {
        // Updating Histroy bits
        in_bounds_time_history = in_bounds_time_history << 1 | 1;
        // in_bounds_history = in_bounds_history << 1| BOUND_FREQ(freq, h_nextState_exp.freq, h_nextState_exp.freq_lb);
        in_bounds_history = in_bounds_history << 1 | (THRESH_FREQ(freq, h_nextState_exp.freq) || THRESH_FREQ(freq, h_nextState_exp.freq_lb));

        // if Skew time not out of bounds
        if ((in_bounds_time_history + 1) != 0) {
            // Freq is within Bounds - Move to next_state
            if ((in_bounds_history + 1) == 0) {
                h_nextState = move_into;

                // Reset Histors
                in_bounds_time_history  = 0;
                in_bounds_history       = 0;
            }
        }

        // Time out of bounds - didn't shift in in time
        else {
            h_nextState = move_failed;

            // Reset Histors
            in_bounds_time_history  = 0;
            in_bounds_history       = 0;
        }
    }
}

void nextState_bound (uint16_t freq, Decoder_FSM_Val move_into, Decoder_FSM_Val move_failed) {
    h_nextState = h_currState;

    // If not within bound - freq changed to next_state freq
    if (!(BOUND_FREQ(freq, h_currState.freq, h_currState.freq_lb))) {
        // Updating Histroy bits
        in_bounds_time_history = in_bounds_time_history << 1 | 1;
        in_bounds_history = in_bounds_history << 1 | (THRESH_FREQ(freq, h_nextState_exp.freq) || THRESH_FREQ(freq, h_nextState_exp.freq_lb));

        // if Skew time not out of bounds
        if ((in_bounds_time_history + 1) != 0) {
            // Freq is within Bounds - Move to next_state
            if ((in_bounds_history + 1) == 0) {
                h_nextState = move_into;

                // Reset Histors
                in_bounds_time_history  = 0;
                in_bounds_history       = 0;
            }
        }

        // Time out of bounds - didn't shift in in time
        else {
            h_nextState = move_failed;

            // Reset Histors
            in_bounds_time_history  = 0;
            in_bounds_history       = 0;

        }
    }
}

void nextState_intoBoundRange (uint16_t freq, Decoder_FSM_Val move_into, Decoder_FSM_Val move_failed) {
    h_nextState = h_currState;

    //printf("BOUND__RANGE_FREQ: %d\n", BOUND_RANGE_FREQ(freq, h_nextState_exp.freq, h_nextState_exp.freq_lb));

    // If not within bound - freq changed to next_state freq
    if (!(THRESH_FREQ(freq, h_currState.freq))) {
        // Updating Histroy bits
        in_bounds_time_history = in_bounds_time_history << 1 | 1;
        in_bounds_history = in_bounds_history << 1 | BOUND_RANGE_FREQ(freq, h_nextState_exp.freq, h_nextState_exp.freq_lb);

        // if Skew time not out of bounds
        if ((in_bounds_time_history + 1) != 0) {
            // Freq is within Bounds - Move to next_state
            if ((in_bounds_history + 1) == 0) {
                h_nextState = move_into;

                // Reset Histors
                in_bounds_time_history  = 0;
                in_bounds_history       = 0;
            }
        }

        // Time out of bounds - didn't shift in in time
        else {
            h_nextState = move_failed;

            // Reset Histors
            in_bounds_time_history  = 0;
            in_bounds_history       = 0;

        }
    }
}

void nextState_boundRange (uint16_t freq, Decoder_FSM_Val move_into, Decoder_FSM_Val move_failed) {
    h_nextState = h_currState;

    // If not within bound - freq changed to next_state freq
    if (!(BOUND_RANGE_FREQ(freq, h_currState.freq, h_currState.freq_lb))) {
        // Updating Histroy bits
        in_bounds_time_history = in_bounds_time_history << 1 | 1;
        in_bounds_history = in_bounds_history << 1 | !(BOUND_RANGE_FREQ(freq, h_currState.freq, h_currState.freq_lb));

        // if Skew time not out of bounds
        if ((in_bounds_time_history + 1) != 0) {
            // Freq is within Bounds - Move to next_state
            if ((in_bounds_history + 1) == 0) {
                h_nextState = move_into;

                // Reset Histors
                in_bounds_time_history  = 0;
                in_bounds_history       = 0;
            }
        }

        // Time out of bounds - didn't shift in in time
        else {
            h_nextState = move_failed;

            // Reset Histors
            in_bounds_time_history  = 0;
            in_bounds_history       = 0;
        }
    }
}
