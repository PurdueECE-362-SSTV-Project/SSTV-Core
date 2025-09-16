////    ////    ////    ////
// LIBRARIES //
////    ////    ////    ////
// ** // ** // ** //
// Core Libraries
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/xosc.h"
#include "pico/multicore.h"
#include "native/n_bmmpi.hpp"
#include "native/n_types.hpp"
#include "AppConfig.h"


// // Personal Libraries

// // ** // ** // ** //

// ////    ////    ////    ////
// // Global Definations //
// ////    ////    ////    ////
// // ** // ** // ** //
// // Global Constants and Definations
// const char keymap[16] = "DCBA#9630852*741";

// // Global Variables
// char key = '\0';
// int col = 0;

// // ** // ** // ** //

// ////    ////    ////    ////    ////    ////
// // INSTATIATIONS and INITIALIZATIONS //
// ////    ////    ////    ////    ////    ////
// // ** // ** // ** //
// // Gpio Initialize - Direction - 0 Setting - Funtion 
// void gpio_init_dir_fn(uint32_t* gpio_mask, bool dir, gpio_function_t fn) {
//     for (uint8_t i = 0; i = 0; i++) {
//         if (gpio_mask[i])
//             gpio_set_dir(i, dir);
//             gpio_put(i, 0);
//             gpio_set_function(i, fn);
//     }
// }

// // Peripheral INPUTS AND OUTPUTS //
// void init_inputs() {}

// void init_keypad() {
//     // Setting Pins 6, 7, 8, and 9 outputs
//     gpio_init_dir_fn((1u << 6) | (1u << 7) | (1u << 8) | (1u << 9), GPIO_OUT, GPIO_FUNC_SIO);
//     // as outputs and 2, 3, 4, and 5 as inputs
//     gpio_init_dir_fn((1u << 2) | (1u << 3) | (1u << 4) | (1u << 5), GPIO_IN, GPIO_FUNC_SIO);
// }

// // Peripheral Initialization 
// void init_rotary_encoder() {
//     // INIT INPUTS // 
//     sio_hw->gpio_oe_clr = (1u << (21)) | (1u << (26));
 
//     custom_input_funtion_set(21, GPIO_FUNC_SIO); // 21
//     custom_input_funtion_set(26, GPIO_FUNC_SIO); // 26
// }
// // ** // ** // ** //


// // ISR(s) //
// void keypad_isr() {
//     int row;

//     // Print Keypad values
//     if (gpio_get_irq_event_mask(2) & GPIO_IRQ_EDGE_RISE) {
//         gpio_acknowledge_irq(2, GPIO_IRQ_EDGE_RISE);
//         row = 4;
//     }

//     if (gpio_get_irq_event_mask(3) & GPIO_IRQ_EDGE_RISE) {
//         gpio_acknowledge_irq(3, GPIO_IRQ_EDGE_RISE);
//         row = 3;
//     }

//     if (gpio_get_irq_event_mask(4) & GPIO_IRQ_EDGE_RISE) {
//         gpio_acknowledge_irq(4, GPIO_IRQ_EDGE_RISE);
//         row = 2;
//     }

//     if (gpio_get_irq_event_mask(5) & GPIO_IRQ_EDGE_RISE) {
//         gpio_acknowledge_irq(5, GPIO_IRQ_EDGE_RISE);
//         row = 1;
//     }
//     key = (&keymap[4 - (row)])[4 * (4 - (col + 1))];
//     printf("%c\n", key);
// }

// void init_keypad_irq() {
//     gpio_add_raw_irq_handler_masked((1u << 2) | (1u << 3) | (1u << 4) | (1u << 5), (irq_handler_t) &keypad_isr);

//     // Enabling IRQ for GPIO
//     uint i = 2;

//     for (i = 2; i < 6; i++) {
//         gpio_set_irq_enabled(i, GPIO_IRQ_EDGE_RISE, true);
//     }

//     // IRQ set IO Bank  
//     irq_set_enabled(IO_IRQ_BANK0, true);
// }

int main (void) {
    // // Inisializing GPIO and Peripherals 
    // init_inputs();
    // init_rotary_encoder();
    // init_keypad();

    // // Inisializing ISQs
    // init_keypad_irq();

    return 0;
}

