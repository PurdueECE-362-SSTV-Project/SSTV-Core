#ifndef ILI9341_C_H_
#define ILI9341_C_H_

#include <stdio.h>

// Pico Libraries
#include "pico/stdlib.h"
#include "hardware/spi.h"

// Custom
#include "ili9341hw.h"

// Public globals mirroring original externs
extern volatile uint16_t* imageBuffer;        // Framebuffer: PIX_WIDTH x PIX_HEIGHT, RGB565 big-endian

// image buffer 2D reference
#define PIX(x,y) ((size_t)((PIX_HEIGHT - 1) - y) * (size_t)PIX_WIDTH + (size_t)(x))

// Initialize framebuffer allocation
void init_imageBuffer(void);
void display_init_dma();
void dma_display_irq(void);

// TFT Pin Values
#define PIN_SDI     19
#define PIN_CS      17
#define PIN_SCK     18
#define PIN_DC      20
#define PIN_nRESET  16
#define PIN_SDO     36
#define PIN_LED     23

// Initialize display and SPI/GPIO, choose orientation
void ili9341_init(spi_inst_t* pspi_port,
                  int spi_clock_freq,
                  uint8_t gpio_MISO,
                  uint8_t gpio_CS,
                  uint8_t gpio_SCK,
                  uint8_t gpio_MOSI,
                  uint8_t gpio_RS,
                  uint8_t gpio_DC,
                  uint8_t led,
                  bool portrait);


void ILI9341_Write8_Prepare();
void ILI9341_Write8_End();

void ILI9341_Write16_Prepare();
void ILI9341_Write16_End();


// Orientation and geometry getters
bool ili9341_isPortrait(void);
uint16_t ili9341_getWidth(void);
uint16_t ili9341_getHeight(void);

// RGB packers
uint16_t ili9341_RGBto16bit(uint8_t r, uint8_t g, uint8_t b);
uint16_t ili9341_RGBAto16bit(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

// 2D drawing primitives (render to framebuffer)
void ili9341_drawRect(uint16_t x, uint16_t y, uint16_t wid, uint16_t hei, uint16_t color);
void ili9341_drawBitmap16(uint16_t x, uint16_t y, uint16_t wid, uint16_t hei, const uint16_t* bitmap);
void ili9341_drawBitmap1(uint16_t x, uint16_t y, uint16_t wid, uint16_t hei, const uint8_t* bitmap, uint16_t fg, uint16_t bg);
void ili9341_drawBitmap32(uint16_t x, uint16_t y, uint16_t wid, uint16_t hei, const uint32_t* bitmap);
void ili9341_drawBitmapRGB8(uint16_t x, uint16_t y, uint16_t wid, uint16_t hei, const uint8_t* bitmap, bool whiteIsBlack);

// Text rendering (uses PubSansBlack font from original include)
int ili9341_putChar(char c, uint16_t x, uint16_t y, uint16_t fg, uint16_t bg);
int ili9341_putStr(const char* s, uint16_t x, uint16_t y, uint16_t fg, uint16_t bg);

void ili9341_clear(void);
void ili9341_test(void);

void ILI9341_write16ImageBuffer(void);
void ILI9341_write16ImageBuffer_Test(void);
void ILI9341_write8ImageBuffer(void);

// Fill framebuffer with a constant 16-bit color
void ILI9341_setScreenColour(uint16_t);

#endif
