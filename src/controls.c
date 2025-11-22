#include <stdio.h>
#include "pico/stdlib.h"

#include "controls.h"
#include "input_control.h"
#include "controls.h"

static volatile int16_t rotary_delta = 0;
volatile bool rotary_switch_flag = false;
volatile bool rotary_isr_flag = false;
volatile bool frontbutton1_flag = false;
volatile bool frontbutton2_flag = false;
volatile bool frontbutton3_flag = false;
volatile uint32_t last_rot_sw_time = 0;

volatile uint16_t frequency10x = 911;
volatile uint16_t volume_level = 15; // Default volume level
volatile uint16_t freq_flag = 0x00;

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

  gpio_init(FRONTBUTTON1);
  gpio_set_dir(FRONTBUTTON1, GPIO_IN);
  gpio_pull_up(FRONTBUTTON1);

  gpio_init(FRONTBUTTON2);
    gpio_set_dir(FRONTBUTTON2, GPIO_IN);
    gpio_pull_up(FRONTBUTTON2);

    gpio_init(FRONTBUTTON3);
    gpio_set_dir(FRONTBUTTON3, GPIO_IN);
    gpio_pull_up(FRONTBUTTON3);

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


void rotary_logic(uint16_t freq_flag) {
    unsigned char result = rotary_state();

    if (result & DIR_CW) {
        if((!freq_flag)) {
            // Not in frequency mode, or both frequency modes selected: adjust volume
            if (volume_level < 15)
                volume_level++;
            printf("Volume = %d\n", volume_level);
        }
        else if ((freq_flag & 0x01) && (frequency10x < 1080)) {
            frequency10x += 1;      // +0.1 MHz
            printf("Frequency = %d\n", frequency10x);
        }
        else if ((freq_flag & 0x02) && (frequency10x < 1070)) {
            frequency10x += 10;     // +1 MHz
            printf("Frequency = %d\n", frequency10x);
        }
    }
    else if (result & DIR_CCW) {
        if((!freq_flag)) {
            // Not in frequency mode, or both frequency modes selected: adjust volume
            if (volume_level > 0)
                volume_level--;
            printf("Volume = %d\n", volume_level);
        }
        else if ((freq_flag & 0x01) && (frequency10x > 761)) {
            frequency10x -= 1;      // -0.1 MHz
            printf("Frequency = %d\n", frequency10x);
        }
        else if ((freq_flag & 0x02) && (frequency10x > 770)) {
            frequency10x -= 10;     // -1 MHz
            printf("Frequency = %d\n", frequency10x);

        }
    }
}


void set_volume() {
    uint16_t regs[16];
    //rf_init();
    //rf_read_register(regs);
}

void rotary_switch_logic(uint16_t frequency_10x, uint16_t volume) {
    rf_tune(frequency_10x); //tune to frequency MHz
    set_volume();
    printf("Frequency set to %d, Volume set to %d\n", frequency_10x, volume);
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

    events = gpio_get_irq_event_mask(FRONTBUTTON1);
    if (events & GPIO_IRQ_EDGE_FALL) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - last_rot_sw_time > ROT_SW_DEBOUNCE_MS) {
            frontbutton1_flag = true;
            freq_flag = (freq_flag +1) % 3;
            last_rot_sw_time = now;
        }
        gpio_acknowledge_irq(FRONTBUTTON1, GPIO_IRQ_EDGE_FALL);
    }

    events = gpio_get_irq_event_mask(FRONTBUTTON2);
    if (events & GPIO_IRQ_EDGE_FALL) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - last_rot_sw_time > ROT_SW_DEBOUNCE_MS) {
            frontbutton2_flag = true;
            last_rot_sw_time = now;
        }
        gpio_acknowledge_irq(FRONTBUTTON2, GPIO_IRQ_EDGE_FALL);
    }

    events = gpio_get_irq_event_mask(FRONTBUTTON3);
    if (events & GPIO_IRQ_EDGE_FALL) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - last_rot_sw_time > ROT_SW_DEBOUNCE_MS) {
            frontbutton3_flag = true;
            last_rot_sw_time = now;
        }
        gpio_acknowledge_irq(FRONTBUTTON3, GPIO_IRQ_EDGE_FALL);
    }
}



void init_rotary_irq() {
    gpio_add_raw_irq_handler(
        (1u << ROTARY_CLK_PIN) | (1u << ROTARY_DT_PIN) | (1u << ROTARY_SW_PIN) | (1u << FRONTBUTTON1) | (1u << FRONTBUTTON2) | (1u << FRONTBUTTON3),
        rotary_isr
    );

    gpio_set_irq_enabled(ROTARY_CLK_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);
    gpio_set_irq_enabled(ROTARY_DT_PIN,  GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);
    gpio_set_irq_enabled(ROTARY_SW_PIN,  GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(FRONTBUTTON1, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(FRONTBUTTON2, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(FRONTBUTTON3, GPIO_IRQ_EDGE_FALL, true);

    irq_set_enabled(IO_IRQ_BANK0, true);

    printf("Rotary Encoder IRQs Initialized\n");
}
