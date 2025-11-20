#ifndef SSTV_CONTROLS_H
#define SSTV_CONTROLS_H

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/irq.h"

#define ROTARY_CLK_PIN 34
#define ROTARY_DT_PIN  33
#define ROTARY_SW_PIN  32

#define FRONTBUTTON1 12
#define FRONTBUTTON2 13
#define FRONTBUTTON3 14

#define ROT_SW_DEBOUNCE_MS 300

static uint8_t state = 0;
extern volatile bool rotary_isr_flag;
extern volatile bool rotary_switch_flag;

// Values returned by 'process'
// No complete step yet.
#define DIR_NONE 0x0
// Clockwise step.
#define DIR_CW 0x10
// Anti-clockwise step.
#define DIR_CCW 0x20

#define R_START 0x0

#define R_CCW_BEGIN 0x1
#define R_CW_BEGIN 0x2
#define R_START_M 0x3
#define R_CW_BEGIN_M 0x4
#define R_CCW_BEGIN_M 0x5

extern const unsigned char ttable[6][4];

void init_pushbuttons();
void pushbutton_isr();
void init_pushbutton_irq();

#endif
