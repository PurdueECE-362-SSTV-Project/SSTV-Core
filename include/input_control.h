#ifndef INPUT_CONTROL_
#define INPUT_CONTROL_

#include <stdio.h>

// Analog Input //
#define SI4703_ADDR 0x10
#define RF_I2C_SDIO 36
#define RF_I2C_SCLK 37
#define RF_RST 40
#define ADC_PIN 41

void init_adc();
void init_dma();
void dma_handler();

void rf_init();
void rf_init_i2c();

void rf_tune(uint16_t);

void rf_write_register(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);
void rf_read_register(uint16_t *);
void rf_print_registers(uint16_t *);
void rf_set_volume(uint16_t);

// Controls //
#define ROTARY_CLK_PIN (uint) 34
#define ROTARY_DT_PIN  (uint) 33
#define ROTARY_SW_PIN  (uint) 32

#define ROT_SW_DEBOUNCE_MS 300

// Values returned by 'process'
// No complete step yet.
#define DIR_NONE 0x0
// Clockwise step.
#define DIR_CW 0x10
// Anti-clockwise step.
#define DIR_CCW 0x20

#define R_START 0x0

#define R_CCW_BEGIN 0x1
#define R_CW_BEGIN 0x2
#define R_START_M 0x3
#define R_CW_BEGIN_M 0x4
#define R_CCW_BEGIN_M 0x5

void init_rotary_irq();
void init_rotary_encoder();

void rotary_isr();
void rotary_sw_isr();

void rotary_logic(uint16_t);
void rotary_switch_logic(u_int16_t, uint16_t);
unsigned char rotary_state();

#endif