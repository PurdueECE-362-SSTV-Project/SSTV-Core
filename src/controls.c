#include <stdio.h>
#include "pico/stdlib.h"

#include "controls.h"

#include "controls.h"

static volatile int16_t rotary_delta = 0;
volatile bool rotary_switch_flag = false;
volatile bool rotary_isr_flag = false;
volatile uint32_t last_rot_sw_time = 0;

const unsigned char ttable[6][4] = {
  {R_START_M,            R_CW_BEGIN,     R_CCW_BEGIN,  R_START},
  {R_START_M | DIR_CCW, R_START,        R_CCW_BEGIN,  R_START},
  {R_START_M | DIR_CW,  R_CW_BEGIN,     R_START,      R_START},
  {R_START_M,            R_CCW_BEGIN_M,  R_CW_BEGIN_M, R_START},
  {R_START_M,            R_START_M,      R_CW_BEGIN_M, R_START | DIR_CW},
  {R_START_M,            R_CCW_BEGIN_M,  R_START_M,    R_START | DIR_CCW},
};


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

  gpio_init(13);
  gpio_set_dir(13, GPIO_IN);
  gpio_pull_up(13);

  state = R_START;
}

unsigned char rotary_state() {
    bool clk = gpio_get(ROTARY_CLK_PIN);
    bool dt = gpio_get(ROTARY_DT_PIN);

    unsigned char pin_state = (clk << 1) | dt;

    state = ttable[state & 0x07][pin_state];

    // Return the direction if a complete step has been made
    return state & 0x30;
}


void rotary_logic() {
    unsigned char result = rotary_state();

    if (result & DIR_CW) {
        rotary_delta++;
        printf("CW, delta=%d\n", rotary_delta);
    }
    else if (result & DIR_CCW) {
        rotary_delta--;
        printf("CCW, delta=%d\n", rotary_delta);
    }
}


void rotary_switch_logic(){
  printf("Rotary switch pressed\n");
}

void rotary_isr() {
    uint32_t events;

    events = gpio_get_irq_event_mask(ROTARY_CLK_PIN);
    if (events & (GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL)) {
        rotary_isr_flag = true;
        gpio_acknowledge_irq(ROTARY_CLK_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL);
    }

    events = gpio_get_irq_event_mask(ROTARY_DT_PIN);
    if (events & (GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL)) {
        rotary_isr_flag = true;
        gpio_acknowledge_irq(ROTARY_DT_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL);
    }

    events = gpio_get_irq_event_mask(ROTARY_SW_PIN);
    if (events & GPIO_IRQ_EDGE_FALL) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - last_rot_sw_time > ROT_SW_DEBOUNCE_MS) {
            rotary_switch_flag = true;
            last_rot_sw_time = now;
        }
        gpio_acknowledge_irq(ROTARY_SW_PIN, GPIO_IRQ_EDGE_FALL);
    }

    events = gpio_get_irq_event_mask(13);
    if (events & GPIO_IRQ_EDGE_FALL) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - last_rot_sw_time > ROT_SW_DEBOUNCE_MS) {
            printf("Pushbutton pressed (from rotary_isr)\n");
            last_rot_sw_time = now;
        }
        gpio_acknowledge_irq(13, GPIO_IRQ_EDGE_FALL);
    }
}



void init_rotary_irq() {
    gpio_add_raw_irq_handler(
        (1u << ROTARY_CLK_PIN) | (1u << ROTARY_DT_PIN) | (1u << ROTARY_SW_PIN) | (1u << 13),
        rotary_isr
    );

    gpio_set_irq_enabled(ROTARY_CLK_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);
    gpio_set_irq_enabled(ROTARY_DT_PIN,  GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);
    gpio_set_irq_enabled(ROTARY_SW_PIN,  GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(13, GPIO_IRQ_EDGE_FALL, true);

    irq_set_enabled(IO_IRQ_BANK0, true);

    printf("Rotary Encoder IRQs Initialized\n");
}