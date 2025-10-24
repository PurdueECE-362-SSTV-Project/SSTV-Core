#ifndef _DECODER_FSM
#define _DECODER_FSM

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef enum DECODING_FSM {
    IDLE, TRANS_ERR,
    VIS_SYNC_1, VIS_SYNC_HOLD, VIS_SYNC_2, 
    START_BIT, VIS_CODE, PARITY_BIT, STOP_BIT,
    TRANS_READ
}  decoding_fsm_t;

#endif
