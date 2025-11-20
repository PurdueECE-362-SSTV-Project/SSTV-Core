#ifndef _USER_INPUT
#define _USER_INPUT

#include <stdio.h>

// Unit Conversions
#define MUS_TO_MS   1000

// GPIO Funtions
void gpio_init_dir_fn(uint32_t, bool, gpio_function_t);

void print_binary(uint64_t, int);

/// Typedef Funtions
void print_current_gpio_hi_state (void);
void print_current_gpio_state (void);

#endif