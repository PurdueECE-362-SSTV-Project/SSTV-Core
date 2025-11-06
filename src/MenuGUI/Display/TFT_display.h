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

#define PIN_SDI    19
#define PIN_CS     17
#define PIN_SCK    18
#define PIN_DC     16
#define PIN_nRESET 20

#endif