#include <stdio.h>
#include "pico/stdlib.h"

#include "custom_funtions.h"

// Print Funtions
void print_uint32_to_binary (uint32_t input) {
    for (int i = 31; i >= 0; i--) {
        printf("%d", (bool) (input & (1 << i)));
    }
}

void print_uint8_to_binary (uint8_t input) {
    for (int i = 7; i >= 0; i--) {
        printf("%d", (bool) (input & (1 << i)));
    }
}

// Print states for gpio_hi_in 
void print_current_gpio_hi_state (void) {
    printf("Pins: ");
    print_uint32_to_binary(sio_hw->gpio_hi_in);
    printf("\n");
}

// Print states for gpio_in 
void print_current_gpio_state (void) {
    printf("Pins: ");
    for (int i = 31; i >= 0; i--) {
        printf("%d", (bool) (sio_hw->gpio_in & (1 << i)));
    }
    printf("\n");
}

// Gpio Initialize - Direction - 0 Setting - Funtion
void gpio_init_dir_fn(uint32_t gpio_mask, bool dir, gpio_function_t fn) {
    for (uint8_t i = 0; i < 32; i++) {
        if (gpio_mask & (1 << i)) {
            gpio_set_dir(i, dir);
            gpio_put(i, 0);
            gpio_set_function(i, fn);
        }
    }
}