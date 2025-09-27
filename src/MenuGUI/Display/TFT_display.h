#ifndef _TFT_DISPLAY
#define _TFT_DISPLAY

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <stdio.h>
#include <string.h>
#include <math.h>   

// Custom Includes
#include "lcd.h"

// Funtion Definations
Picture* load_image(const char* image_data);
void free_image(Picture* pic);
void init_spi_lcd(void);

// Testing Funtions
void TFT_rickRoll (void);

#define PIN_SDI    23
#define PIN_CS     21
#define PIN_SCK    22
#define PIN_DC     24
#define PIN_nRESET 25

#endif