#include <stdio.h>
#include "pico/stdlib.h"

#include "../include/test.h"


void config_pin() {
    gpio_set_dir(23, true);
}

void to_high() {
    sio_hw->gpio_set = 1 << 23;
}

void to_low() {
    sio_hw->gpio_clr = 1 << 23;
}