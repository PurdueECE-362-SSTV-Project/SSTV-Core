#ifndef _USER_INPUT
#define _USER_INPUT

// Core Libraries
#include <stdio.h>
#include "pico/stdlib.h"

#include "file_locations.h"

// Keypad Definations
const char keymap[17] = "DCBA#9630852*741";

#define DELAY_DRIVE_READ     MUS_TO_MS * 25                     // 25 ms

#define SECOND                  MUS_TO_MS * 1000                // 1 s
#define SECOND_1                MUS_TO_MS * 1100                // 1.1 s

#define ALARM_NUM_DRIVE 0
#define ALARM_NUM_READ  1

#define ALARM_IRQ_DRIVE TIMER0_IRQ_0
#define ALARM_IRQ_READ  TIMER0_IRQ_1

#define KEYPAD_OUT_MASK (uint32_t) ((1u << 6) | (1u << 7) | (1u << 8) | (1u << 9))

#define ROW_GP2     4
#define ROW_GP3     3
#define ROW_GP4     2
#define ROW_GP5     1

//Function Definations
void keypad_isr(void);
void init_keypad_irq(void);
void keypad_drive_column();
void keypad_isr();

#endif