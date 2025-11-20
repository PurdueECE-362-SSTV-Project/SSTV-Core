#include <stdio.h>

// Pico Libraries
#include "pico/stdlib.h"

// Custom
#include "custom_funtions.h"

// Print Funtions
void print_binary(uint64_t v, int bits) {
    if (bits <= 0) return;
    for (int i = bits - 1; i >= 0; --i) {
        printf("%c", ( (v >> i) & 1ULL ) ? '1' : '0');
    }
    printf("\n");
}

// Print states for gpio_hi_in 
void print_current_gpio_hi_state (void) {
    printf("Pins: ");
    print_binary(sio_hw->gpio_hi_in, 32);
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