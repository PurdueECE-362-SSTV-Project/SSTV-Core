#ifndef _USER_INPUT
#define _USER_INPUT

////    ////    ////    ////
// LIBRARIES //
////    ////    ////    ////
// ** // ** // ** //
// Core Libraries
#include <stdio.h>
#include "pico/stdlib.h"

// Touchscreen Libraries - SPI
#include "../lib/Adafruit-GFX-Library-1.12.1/Adafruit_GFX.h"        
#include "../lib/TFTLCD-Library-1.0.3/Adafruit_TFTLCD.h"  
#include "../lib/Adafruit_TouchScreen-1.1.6/TouchScreen.h"

////    ////    ////    ////
// Global Definations //
////    ////    ////    ////
// ** // ** // ** //
// Global Constants and Definations
const char keymap[16] = "DCBA#9630852*741";

// Global Variables
char key = '\0';
int col = 0;

////    ////    ////    ////    ////    ////
// INSTATIATIONS and INITIALIZATIONS //
////    ////    ////    ////    ////    ////
void keypad_isr(void);
void init_keypad_irq(void);

// ** // ** // ** //
// ISR(s) //
void keypad_isr() {
    int row;

    // Print Keypad values
    if (gpio_get_irq_event_mask(2) & GPIO_IRQ_EDGE_RISE) {
        gpio_acknowledge_irq(2, GPIO_IRQ_EDGE_RISE);
        row = 4;
    }

    if (gpio_get_irq_event_mask(3) & GPIO_IRQ_EDGE_RISE) {
        gpio_acknowledge_irq(3, GPIO_IRQ_EDGE_RISE);
        row = 3;
    }

    if (gpio_get_irq_event_mask(4) & GPIO_IRQ_EDGE_RISE) {
        gpio_acknowledge_irq(4, GPIO_IRQ_EDGE_RISE);
        row = 2;
    }

    if (gpio_get_irq_event_mask(5) & GPIO_IRQ_EDGE_RISE) {
        gpio_acknowledge_irq(5, GPIO_IRQ_EDGE_RISE);
        row = 1;
    }
    key = (&keymap[4 - (row)])[4 * (4 - (col + 1))];
    printf("%c\n", key);
}

void init_keypad_irq() {
    gpio_add_raw_irq_handler_masked((1u << 2) | (1u << 3) | (1u << 4) | (1u << 5), (irq_handler_t) &keypad_isr);

    // Enabling IRQ for GPIO
    uint i = 2;

    for (i = 2; i < 6; i++) {
        gpio_set_irq_enabled(i, GPIO_IRQ_EDGE_RISE, true);
    }

    // IRQ set IO Bank  
    irq_set_enabled(IO_IRQ_BANK0, true);
}

#endif