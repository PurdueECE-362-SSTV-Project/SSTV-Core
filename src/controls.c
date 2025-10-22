#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/i2c.h"



#define ROTARY_CLK_PIN 34
#define ROTARY_DT_PIN  33
#define ROTARY_SW_PIN  32




void init_rotary_encoder() {
  gpio_init(ROTARY_CLK_PIN);
  gpio_set_dir(ROTARY_CLK_PIN, GPIO_IN);
  gpio_pull_up(ROTARY_CLK_PIN);

  gpio_init(ROTARY_DT_PIN);
  gpio_set_dir(ROTARY_DT_PIN, GPIO_IN);
  gpio_pull_up(ROTARY_DT_PIN);

  gpio_init(ROTARY_SW_PIN);
  gpio_set_dir(ROTARY_SW_PIN, GPIO_IN);
  gpio_pull_up(ROTARY_SW_PIN);
}

void init_keypad() {
    // Setting Pins 6, 7, 8, and 9 outputs
    gpio_init_dir_fn((1u << 6) | (1u << 7) | (1u << 8) | (1u << 9), GPIO_OUT, GPIO_FUNC_SIO);
    // as outputs and 2, 3, 4, and 5 as inputs
    gpio_init_dir_fn((1u << 2) | (1u << 3) | (1u << 4) | (1u << 5), GPIO_IN, GPIO_FUNC_SIO);
}