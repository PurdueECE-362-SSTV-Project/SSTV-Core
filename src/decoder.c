#include <stdio.h>
#include "pico/stdlib.h"

#include "Ili9341.h"
#include "decoder.h"
#include "custom_funtions.h"

// ASYNC FSM Vars
static const Decoder_FSM_Val h_idle      = {.State  = IDLE,        .freq = 0,      .freq_lb = 0,    .diff_time = 0};
static const Decoder_FSM_Val h_sync1     = {.State  = SYNC1,       .freq = 1900,   .freq_lb = 0,    .diff_time = 300};
static const Decoder_FSM_Val h_hold      = {.State  = SYNC_HOLD,   .freq = 1200,   .freq_lb = 0,    .diff_time = 10};
static const Decoder_FSM_Val h_sync2     = {.State  = SYNC2,       .freq = 1900,   .freq_lb = 0,    .diff_time = 300};
static const Decoder_FSM_Val h_start     = {.State  = START_BIT,   .freq = 1200,   .freq_lb = 0,    .diff_time = 30};
static const Decoder_FSM_Val h_code      = {.State  = CODE,        .freq = 1300,   .freq_lb = 1100, .diff_time = 240};
static const Decoder_FSM_Val h_stop      = {.State  = STOP_BIT,    .freq = 1200,   .freq_lb = 0,    .diff_time = 30};
static const Decoder_FSM_Val h_decode    = {.State  = DECODE_MODE, .freq = 2350,   .freq_lb = 1150, .diff_time = 300};     // Go into decoding mode

// Colour FSM Vars
static const Colour_Decoder_Val c_line  = {.freqCol = 2300,     .freqCol_lb = 1500,     .freq = 2300 + COL_TH_UP,    .freq_lb = 1500 + COL_TH_LW,      .freqDiv = 2300.0 - 1500.0};

// SSTV MODE Vars
static const sstv_mode_t Robot36     =  {.decMode = ROBOT_36,    .color = YCRCB, .row = 320,     .col = 240,     .tranTime = 36,     .lineTime = 45.0    / 1000.0,    .colorTime =  45.0 / 3000.0,        .colTimeSample = (45.0 * SFREQ) / 1000.0,       .colSampleL = ((45.0 * SFREQ) / (1000.0 * 320.0)),         .pixelsLost = 0,   .colFrac = ((45.0 * SFREQ) / (1000.0 * 320.0)) - (uint8_t) ((45.0 * SFREQ) / (1000.0 * 320.0)),         .lineS = 10.5,  .colorS = 4.5, .format = 1};
static const sstv_mode_t Robot72     =  {.decMode = ROBOT_72,    .color = YCRCB, .row = 320,     .col = 240,     .tranTime = 72,     .lineTime = 92.0    / 1000.0,    .colorTime =  92.0 / 3000.0,        .colTimeSample = (92.0 * SFREQ) / 1000.0,       .colSampleL = ((92.0 * SFREQ) / (1000.0 * 320.0)),         .pixelsLost = 0,   .colFrac = ((92.0 * SFREQ) / (1000.0 * 320.0)) -  (uint8_t) ((92.0 * SFREQ) / (1000.0 * 320.0)),        .lineS = 12.0,  .colorS = 6.0, .format = 2};
static const sstv_mode_t Martin1     =  {.decMode = MARTIN_1,    .color = GBR,   .row = 320,     .col = 256,     .tranTime = 114,    .lineTime = 146.432 / 1000.0,    .colorTime =  146.432 / 3000.0,     .colTimeSample = (146.432 * SFREQ) / 1000.0,    .colSampleL = ((146.432 / 320.0) * (SFREQ / 1000.0)),      .pixelsLost = (COLOUR_HISTORY_NUM / ((146.432 / 320.0) * (SFREQ / 1000.0))),   .colFrac = ((146.432 * SFREQ) / (1000.0 * 320.0)) - (uint8_t) ((146.432 * SFREQ) / (1000.0 * 320.0)),   .lineS = 4.862, .colorS = 0.0, .format = 0};
static const sstv_mode_t Martin2     =  {.decMode = MARTIN_2,    .color = GBR,   .row = 256,     .col = 160,     .tranTime = 58,     .lineTime = 73.216  / 1000.0,    .colorTime =  73.216 / 3000.0,      .colTimeSample = (73.216 * SFREQ) / 1000.0,     .colSampleL = ((73.216 * SFREQ) / (1000.0 * 256.0)),       .pixelsLost = 0,   .colFrac = ((73.216 * SFREQ) / (1000.0 * 256.0)) - (uint8_t) ((73.216 * SFREQ) / (1000.0 * 256.0)),     .lineS = 4.862, .colorS = 0.0, .format = 0};
static const sstv_mode_t Scottie2    =  {.decMode = SCOTTIE_2,   .color = GBR,   .row = 256,     .col = 160,     .tranTime = 71,     .lineTime = 88.064  / 1000.0,    .colorTime =  8.064 / 3000.0,       .colTimeSample = (8.064 * SFREQ) / 1000.0,      .colSampleL = ((8.064 *  SFREQ) / (1000.0 * 256.0)),       .pixelsLost = 0,   .colFrac = ((8.064 * SFREQ) / (1000.0 * 256.0)) - (uint8_t) ((8.064 * SFREQ) / (1000.0 * 256.0)),       .lineS = 9.0,   .colorS = 0.0, .format = 0};
static const sstv_mode_t PD50        =  {.decMode = PD_50,       .color = YCRCB, .row = 320,     .col = 240,     .tranTime = 50,     .lineTime = 61.0133 / 1000.0,    .colorTime =  61.0133 / 3000.0,     .colTimeSample = (61.0133 * SFREQ) / 1000.0,    .colSampleL = ((61.0133 * SFREQ) / (1000.0 * 320.0)),      .pixelsLost = 0,   .colFrac = ((61.0133 * SFREQ) / (1000.0 * 320.0)) - (uint8_t) ((61.0133 * SFREQ) / (1000.0 * 320.0)),   .lineS = 20.0,  .colorS = 0.0, .format = 0};
static const sstv_mode_t PD90        =  {.decMode = PD_90,       .color = YCRCB, .row = 320,     .col = 240,     .tranTime = 90,     .lineTime = 113.493 / 1000.0,    .colorTime =  113.493 / 3000.0,     .colTimeSample = (113.493 * SFREQ) / 1000.0,    .colSampleL = ((113.493 * SFREQ) / (1000.0 * 320.0)),      .pixelsLost = 0,   .colFrac = ((113.493 * SFREQ) / (1000.0 * 320.0)) - (uint8_t) ((113.493 * SFREQ) / (1000.0 * 320.0)),   .lineS = 20.0,  .colorS = 0.0, .format = 0};
static const sstv_mode_t NULL_mode   =  {.decMode = NULL_type,   .color = GBR,   .row = 0,       .col = 0,       .tranTime = 0,      .lineTime = 0,                   .colorTime =  0,                    .colTimeSample = 0,                             .colSampleL = 0,                                           .pixelsLost = 0,             .colFrac = 0,                                                                                           .lineS = 0,     .colorS = 0,   .format = 0};

// Init vars flag
static bool init_vars = true; 

// VIS Code Vars
static bool new_vis_code = true;
static uint64_t sample_count = 0;

static uint8_t VIS_Code   = 0;                  // Stores the VIS Code 
static uint32_t VIS_CODE_upper = 0;             // VIS Code 0 history
static uint32_t VIS_CODE_lower = 0;             // VIS Code 1 history

static sstv_mode_t curr_VIS_mode = NULL_mode;

// Global FSM Holder
static Decoder_FSM_Val h_prevState     = h_idle;        // PrevState Freq
static Decoder_FSM_Val h_currState     = h_idle;        // CurrState Freq
static Decoder_FSM_Val h_nextState     = h_idle;        // NextState Freq
static Decoder_FSM_Val h_nextState_exp = h_idle;        // NextState exp Freq

// History Flags
static uint32_t in_bounds_history  = 0;
static uint64_t in_bounds_time_history = 0;
// History Flags Colour
static uint16_t colour_history = 0;
static uint16_t sync_history = 0;

// Correction Flags
volatile uint8_t pixel_corr = 3;
volatile uint8_t samples_lost_corr = 7;

volatile uint8_t clipping_reduction = 120;
volatile uint8_t underflow_reduction = 50;
#define CLIPPING_RED clipping_reduction
#define UNDERFLOW_RED underflow_reduction

#define sample_read_offset  10

// Easy Grab Macros
#define GET_MODE_WIDTH      curr_VIS_mode.row
#define GET_MODE_HEIGHT     curr_VIS_mode.col
#define GET_LINE_TIME       curr_VIS_mode.lineTime
#define GET_COL_TIME        curr_VIS_mode.colorTime
#define COL_TIME_SAMPLES    curr_VIS_mode.colTimeSample
#define COLOUR_SAMPLE       curr_VIS_mode.colSampleL
#define COLOUR_FRAC         curr_VIS_mode.colFrac

#define PIXELS_LOST        curr_VIS_mode.pixelsLost + pixel_corr
#define SAMPLES_LOST       COLOUR_HISTORY_NUM + samples_lost_corr

// Coordinate vars
static uint16_t curr_x = 0;  
static uint16_t curr_y = 0;

static uint16_t colSec1 = 0;
static uint16_t colSec2 = 0;

static float error_accum = 0;
static bool extra_sam = 0; 

// Colour Decoding Counters
static int colour_count = 0;
static int next_count = 0;
static uint16_t sync_count = 0;

// Colour Decode Flags
static bool in_colour_flag = false;
static bool in_sync_flag = false;
// First enter flags
static bool sync_first_enter = false;
static bool colour_first_enter = false;

void decoder(int16_t in_buff) {
    header_fsm(in_buff);
}

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
                if (!init_vars) init_vars = true;

                nextState_thresh(freq, h_start, h_idle);
            break;

        case (START_BIT):
                // To avoid wasting computation
                if (init_vars) {
                    init_vars = false;          // Dont init more

                    // VIS Code Vars
                    sample_count = 0;
                    VIS_CODE_upper = 0;
                    VIS_CODE_lower = 0;

                    new_vis_code = true;

                    // Colour Decoding Vars
                    colour_count = 0;
                    next_count = -1;           // Smaller than colour count
                    sync_count = 0;

                    colour_history = 0;
                    sync_history = 0;

                    in_colour_flag = false;
                    in_sync_flag = true;

                    sync_first_enter = false;
                    colour_first_enter = false;

                    error_accum = 0;
                    extra_sam   = 0;

                    colSec1 = 0;
                    colSec2 = 0;

                    // Pixel Coorinates
                    curr_x = PIX_WIDTH - 1;
                    curr_y = PIXELS_LOST;
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
            if (!((sample_count +  sample_read_offset) % SAMPLES_PER_BIT)) {
                new_vis_code = true;
                VIS_CODE_upper = VIS_CODE_lower = 0;
            }
            sample_count++;

            break;

        case (STOP_BIT):
                // Print VIS Code bits
                //print_binary(VIS_Code, 8);
                curr_VIS_mode = initVISMode(VIS_Code);

                // To count samples for decode mode
                //sample_count = 0;

                nextState_intoBoundRange(freq, h_decode, h_idle);
            break;

        case (DECODE_MODE):
                // Colour Decoding FSM
                colour_decoder(freq);

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

void colour_decoder(uint16_t freq) {
    // It'll be in either colour or sync
    colour_history  = (colour_history << 1) | BOUND_RANGE_FREQ(freq, c_line.freq, c_line.freq_lb);

    // Colour Mode
    if (in_colour_flag) {
        colour_count++;

        // if Colour First Enter
        if (colour_first_enter) {
            // Print 
            printf("Sync #%d\n", sync_count);

            // Reset Vars
            colour_first_enter = false;
            sync_count = 0;
        }

        // If Sync Enter
        else if (colour_history == 0x0000) {
            in_colour_flag = false;
            in_sync_flag = true;

            // Colour First Enter
            sync_first_enter = true;
        }

        // Colour Decoding Logic    //
        //if (!(colour_count % 50)) printf("curr_x #%d, curr_y #%d\n", curr_x, curr_y);

        // New Sample
        if ((colour_count - next_count) > 0) {
            // Error accum logic
            error_accum += COLOUR_FRAC;
            if (error_accum > 1) { extra_sam = 1; error_accum -= 1;}
            else                 { extra_sam = 0;}

            // Next count increment
            next_count = colour_count + COLOUR_SAMPLE;

            // Colour Decoding Logic
            //                          upper clip                                         lower clip                                          non
            uint16_t freq_adjusted = (freq > c_line.freqCol) ? (freq - CLIPPING_RED) : ((freq < c_line.freqCol_lb) ? (freq + UNDERFLOW_RED) : freq); 
            uint16_t colour64 = (((float) (freq_adjusted - c_line.freqCol_lb)) / c_line.freqDiv) * 64.0;

            // Different colour Sections
            switch((uint8_t) (colour_count / COL_TIME_SAMPLES)) {
                // Green
                case 0: 
                    curr_y++;
                    imageBuffer[PIX(curr_x, curr_y)] = (colour64 << 5);
                    break;

                // Blue
                case 1: 
                    curr_y = ++colSec1;
                    imageBuffer[PIX(curr_x, curr_y)] = (imageBuffer[PIX(curr_x, curr_y)]) | (colour64 >> 1);
                    break;
                
                // Red
                case 2: 
                    curr_y = ++colSec2;
                    imageBuffer[PIX(curr_x, curr_y)] = ((colour64 >> 1) << 11) | (imageBuffer[PIX(curr_x, curr_y)]);
                    break;

                default: 
                    //imageBuffer[PIX(curr_x, curr_y)] = TFT_RED;
                    break;
            } 
        }
        ////    ////   ////   ////  ////
    }

    // Sync Mode
    else {
        sync_count++;

        // If Sync First Enter
        if (sync_first_enter) {
            // Print 
            printf("Colour #%d\n", colour_count);

            // Reset Vars
            sync_first_enter = false;
            colSec1 = 0;
            colSec2 = 0;

            // Updating Coordinates
            colour_count = SAMPLES_LOST;    // History loses first few samples  
            next_count = -1;  
            curr_y = PIXELS_LOST;
            curr_x--;
            //ili9341_drawRect(curr_x, 50, 2, 2, TFT_AQUAMARINE);
        }

        // If Colour Enter
        else if (colour_history == COLOUR_HISTORY) {
            in_colour_flag = true;
            in_sync_flag = false;

            // Colour First Enter
            colour_first_enter = true;
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