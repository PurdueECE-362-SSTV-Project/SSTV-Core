#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Pico Libraries
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"

// Custom
#include "ili9341hw.h"
#include "Ili9341.h"
#include "PubSansBlack.h" 

// Global Variables updated instead of class variables
static spi_inst_t* s_spi = NULL;
static int s_spi_clock = 0;
static uint8_t s_gp_miso = 0;
static uint8_t s_gp_cs   = 0;
static uint8_t s_gp_sck  = 0;
static uint8_t s_gp_mosi = 0;
static uint8_t s_gp_rs   = 0;
static uint8_t s_gp_dc   = 0;
static uint8_t s_gp_led  = 0;
static bool s_portrait   = true;
static uint16_t s_width  = 240;
static uint16_t s_height = 320;

// imageBuffer
volatile uint16_t* imageBuffer = NULL;

// initialize the imageBuffer
void init_imageBuffer(void) {
    imageBuffer = (volatile uint16_t*) calloc(PIX_WIDTH * PIX_HEIGHT, sizeof(uint16_t));
    if (!imageBuffer) {
        printf("malloc failed for framebuffer\n");
    }
}

static inline void ILI9341_CS_Set(int state) {
    asm volatile("nop \n nop \n nop");
    gpio_put(s_gp_cs, state);
    asm volatile("nop \n nop \n nop");
}

static void ILI9341_SetCommand(uint8_t cmd) {
    ILI9341_CS_Set(CS_ENABLE);
    gpio_put(s_gp_dc, 0);
    asm volatile("nop \n nop \n nop");
    spi_write_blocking(s_spi, &cmd, 1);
    gpio_put(s_gp_dc, 1);
    ILI9341_CS_Set(CS_DISABLE);
}

static void ILI9341_CommandParam(uint8_t data) {
    ILI9341_CS_Set(CS_ENABLE);
    spi_write_blocking(s_spi, &data, 1);
    ILI9341_CS_Set(CS_DISABLE);
}

static void ILI9341_SetOutWriting(int sc, int ec, int sp, int ep) {
    // Column address set.
    ILI9341_SetCommand(ILI9341_CASET);
    ILI9341_CommandParam((sc >> 8) & 0xFF);
    ILI9341_CommandParam(sc & 0xFF);
    ILI9341_CommandParam((ec >> 8) & 0xFF);
    ILI9341_CommandParam(ec & 0xFF);

    // Page address set
    ILI9341_SetCommand(ILI9341_PASET);
    ILI9341_CommandParam((sp >> 8) & 0xFF);
    ILI9341_CommandParam(sp & 0xFF);
    ILI9341_CommandParam((ep >> 8) & 0xFF);
    ILI9341_CommandParam(ep & 0xFF);

    // Start writing
    ILI9341_SetCommand(ILI9341_RAMWR);
}

/// @brief Writes a data buffer to display.
/// @param pconfig Control structure of display.
/// @param buffer Data buffer.
/// @param bytes Size of the buffer in bytes.
static void ILI9341_WriteData8(const uint8_t* buf, int bytes) {
    ILI9341_CS_Set(CS_ENABLE);
    spi_write_blocking(s_spi, buf, bytes);
    ILI9341_CS_Set(CS_DISABLE);
}

static void ILI9341_WriteData16(const uint16_t* buf, int bytes) {
    ILI9341_CS_Set(CS_ENABLE);
    spi_set_format(spi0, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    spi_write16_blocking(s_spi, buf, bytes);
    ILI9341_CS_Set(CS_DISABLE);
    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
}

/// @brief Writes an image buffer to display
void ILI9341_writeImageBuffer(void) {
    if (!imageBuffer) return;
    ILI9341_SetOutWriting(0, PIX_WIDTH - 1, 0, PIX_HEIGHT - 1);
    for (int y = PIX_HEIGHT - 1; y >= 0; --y) {
        const uint16_t* row = (const uint16_t*)&imageBuffer[y * PIX_WIDTH];
        ILI9341_WriteData16(row, (int)(PIX_WIDTH));
    }
}

/// @brief Set the entire Screen a colour
void ILI9341_setScreenColour(uint16_t color16) {
    if (!imageBuffer) return;

    for (int i = 0; i < PIX_HEIGHT * PIX_WIDTH; ++i) {
        imageBuffer[i] = color16;
    }
}

static void hw_init(spi_inst_t* pspi_port,
                    int spi_clock_freq,
                    uint8_t gpio_MISO,
                    uint8_t gpio_CS,
                    uint8_t gpio_SCK,
                    uint8_t gpio_MOSI,
                    uint8_t gpio_RS,
                    uint8_t gpio_DC,
                    uint8_t led) {
    s_spi = pspi_port;
    s_spi_clock = spi_clock_freq;
    s_gp_miso = gpio_MISO;
    s_gp_cs   = gpio_CS;
    s_gp_sck  = gpio_SCK;
    s_gp_mosi = gpio_MOSI;
    s_gp_rs   = gpio_RS;
    s_gp_dc   = gpio_DC;
    s_gp_led  = led;

    gpio_init(s_gp_led);
    gpio_set_function(s_gp_led, GPIO_FUNC_PWM);
    pwm_set_gpio_level(s_gp_led, 0xFFFF);
    uint sliceNum = pwm_gpio_to_slice_num(s_gp_led);
    pwm_set_enabled(sliceNum, true);

    spi_init(s_spi, s_spi_clock);
    spi_set_baudrate(s_spi, s_spi_clock);
    gpio_set_function(s_gp_miso, GPIO_FUNC_SPI);
    gpio_set_function(s_gp_sck,  GPIO_FUNC_SPI);
    gpio_set_function(s_gp_mosi, GPIO_FUNC_SPI);

    gpio_init(s_gp_cs);
    gpio_set_dir(s_gp_cs, GPIO_OUT);
    gpio_put(s_gp_cs, 1);

    gpio_init(s_gp_rs);
    gpio_set_dir(s_gp_rs, GPIO_OUT);
    gpio_put(s_gp_rs, 1);

    gpio_init(s_gp_dc);
    gpio_set_dir(s_gp_dc, GPIO_OUT);
    gpio_put(s_gp_dc, 0);

    sleep_ms(10);
    //vTaskDelay(10);
    gpio_put(s_gp_rs, 0);
    sleep_ms(10);
    //vTaskDelay(10);
    gpio_put(s_gp_rs, 1);

    ILI9341_SetCommand(0x01);        //vTaskDelay(10);
    sleep_ms(100);

    ILI9341_SetCommand(ILI9341_GAMMASET);
    // Best mode for this display
    ILI9341_CommandParam(0x04);

    // Positive gamma correction
    ILI9341_SetCommand(ILI9341_GMCTRP1);
    {
        uint8_t data[15] = {0x0f,0x31,0x2b,0x0c,0x0e,0x08,0x4e,0xf1,0x37,0x07,0x10,0x03,0x0e,0x09,0x00};
        ILI9341_WriteData8(data, 15);
    }

    // Negative gamma correction.
    ILI9341_SetCommand(ILI9341_GMCTRN1);
    {
        uint8_t data2[15] = {0x00,0x0e,0x14,0x03,0x11,0x07,0x31,0xc1,0x48,0x08,0x0f,0x0c,0x31,0x36,0x0f};
        ILI9341_WriteData8(data2, 15);
    }

    //MV = 1 - Row/Column Exchange
    //BGR = 1 - BGR Panel?
    //ILI9341_CommandParam(0x48)
    ILI9341_SetCommand(ILI9341_MADCTL);
    if (s_portrait) {
        ILI9341_CommandParam(0x08);
    } else {
        ILI9341_CommandParam(0x68);
    }

    ILI9341_SetCommand(ILI9341_PIXFMT);
    ILI9341_CommandParam(0x55); // 16-bit pixel format

    ILI9341_SetCommand(ILI9341_FRMCTR1);
    ILI9341_CommandParam(0x00);
    ILI9341_CommandParam(0x1B);

    // Set display on
    ILI9341_SetCommand(ILI9341_SLPOUT);
    ILI9341_SetCommand(ILI9341_DISPON);
}

void ili9341_init(spi_inst_t* pspi_port,
                  int spi_clock_freq,
                  uint8_t gpio_MISO,
                  uint8_t gpio_CS,
                  uint8_t gpio_SCK,
                  uint8_t gpio_MOSI,
                  uint8_t gpio_RS,
                  uint8_t gpio_DC,
                  uint8_t led,
                  bool portrait) {
    s_portrait = portrait;
    if (s_portrait) {
        s_width = 240;
        s_height = 320;
    } else {
        s_width = 320;
        s_height = 240;
    }
    hw_init(pspi_port, spi_clock_freq, gpio_MISO, gpio_CS, gpio_SCK, gpio_MOSI, gpio_RS, gpio_DC, led);
}

bool ili9341_isPortrait(void) { return s_portrait; }
uint16_t ili9341_getWidth(void) { return s_width; } 
uint16_t ili9341_getHeight(void) { return s_height; }


uint16_t ili9341_RGBto16bit(uint8_t r, uint8_t g, uint8_t b) {
    r >>= 3; 
    g >>= 2; 
    b >>= 3;
    uint16_t res = (((uint16_t)b & 31) << 11) | (((uint16_t)g & 63) << 5) | ((uint16_t)r & 31);
    return (uint16_t)((res >> 8) | (res << 8));
}

uint16_t ili9341_RGBAto16bit(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    uint16_t red = (uint16_t)(r >> 3);  // 5 bits
    uint16_t grn = (uint16_t)(g >> 2);  // 6 bits
    uint16_t blu = (uint16_t)(b >> 3);  // 5 bits

    if (a != 0xFF) {
        float af = (float)a / 255.0f;
        red = (uint16_t)((float)red * af);
        grn = (uint16_t)((float)grn * af);
        blu = (uint16_t)((float)blu * af);
    }

    uint16_t res = 0;
    res = (uint16_t)((blu & 0x1F) << 3);
    res |= (uint16_t)((red & 0x1F) << 8);

    res |= (uint16_t)   (((grn & 0x38) >> 3) | ((grn & 0x07) << 13));

    return res;
}

void ili9341_drawRect(uint16_t x, uint16_t y, uint16_t wid, uint16_t hei, uint16_t color) {
    if (!imageBuffer) return;
    
    for (int idx_y = y; (idx_y < y + hei) && (idx_y < PIX_HEIGHT); ++idx_y) {
        for (int idx_x = x; (idx_x < x + wid) && (idx_x < PIX_WIDTH); ++idx_x) {
            imageBuffer[PIX(idx_x, idx_y)] = color;
        }
    }
}

void ili9341_drawPixel(uint16_t x, uint16_t y, uint16_t color) {
    if (!imageBuffer) return;
    
    imageBuffer[PIX(x, y)] = color;
}


void ili9341_drawBitmap16(uint16_t x, uint16_t y, uint16_t wid, uint16_t hei, const uint16_t* bitmap) {
    // Check if null
    if (!imageBuffer || !bitmap) return;

    for (int idx_y = y; (idx_y < y + hei) && (idx_y < PIX_HEIGHT); ++idx_y) {
        for (int idx_x = x; (idx_x < x + wid) && (idx_x < PIX_WIDTH); ++idx_x) {
            imageBuffer[PIX(idx_x, idx_y)] = bitmap[(idx_x - x) + ((idx_y - y) * wid)];
        }
    }
}

static void draw_intDrawBitmap1(uint16_t x, uint16_t y, uint16_t wid, uint16_t dataWid, uint16_t hei,
                                 const uint8_t* bitmap, uint16_t fg, uint16_t bg) {
    // Check if null
    if (!imageBuffer || !bitmap) return;

    for (int idx_y = y; (idx_y < y + hei) && (idx_y < PIX_HEIGHT); ++idx_y) {
        for (int idx_x = x; (idx_x < x + wid) && (idx_x < PIX_WIDTH); ++idx_x) {
            uint16_t index =    (uint16_t) ((idx_y - y) * dataWid + (idx_x - x));
            uint16_t byteI =    (uint16_t) (index / 8);
            uint8_t bitI =      (uint8_t)  (index % 8);
            uint8_t mask =      (uint8_t)  (0x80u >> bitI);

            imageBuffer[PIX(idx_x, idx_y)] = ((bitmap[byteI] & mask) ? fg : bg);
        }
    }
}

void ili9341_drawBitmap1(uint16_t x, uint16_t y, uint16_t wid, uint16_t hei, const uint8_t* bitmap, uint16_t fg, uint16_t bg) {
    uint16_t dataWid = wid;
    if ((dataWid % 8) != 0) dataWid = (uint16_t)(8 * (dataWid / 8 + 1));

    draw_intDrawBitmap1(x, y, wid, dataWid, hei, bitmap, fg, bg);
}

void ili9341_drawBitmap32(uint16_t x, uint16_t y, uint16_t wid, uint16_t hei, const uint32_t* bitmap) {
    // Check if null
    if (!imageBuffer || !bitmap) return;

    for (int idx_y = y; (idx_y < y + hei) && (idx_y < PIX_HEIGHT); ++idx_y) {
        for (int idx_x = x; (idx_x < x + wid) && (idx_x < PIX_WIDTH); ++idx_x) {
            uint16_t idx = (uint16_t)((idx_y - y) * wid + (idx_x - x));

            uint8_t a = (uint8_t) ((bitmap[idx] & 0xFF000000u) >> 24);
            uint8_t b = (uint8_t) ((bitmap[idx] & 0x00FF0000u) >> 16);
            uint8_t g = (uint8_t) ((bitmap[idx] & 0x0000FF00u) >> 8);
            uint8_t r = (uint8_t) (bitmap[idx]  & 0x000000FFu);

            imageBuffer[PIX(idx_x, idx_y)] = ili9341_RGBAto16bit(r, g, b, a);
        }
    }
}

void ili9341_drawBitmapRGB8(uint16_t x, uint16_t y, uint16_t wid, uint16_t hei, const uint8_t* bitmap, bool whiteIsBlack) {
    // Check if null
    if (!imageBuffer || !bitmap) return;

    for (int idx_y = y; (idx_y < y + hei) && (idx_y < PIX_HEIGHT); ++idx_y) {
        for (int idx_x = x; (idx_x < x + wid) && (idx_x < PIX_WIDTH); ++idx_x) {
            uint16_t index = (uint16_t)(((idx_y - y) * wid + (idx_x - x)) * 3);

            uint8_t r = bitmap[index + 0];
            uint8_t g = bitmap[index + 1];
            uint8_t b = bitmap[index + 2];

            if (whiteIsBlack && r == 0xFF && g == 0xFF && b == 0xFF) {
                r = g = b = 0x00;
            }

            imageBuffer[PIX(idx_x, idx_y)] = ili9341_RGBto16bit(r, g, b);
        }
    }
}

void ili9341_clear(void) {
    ili9341_drawRect(0, 0, ili9341_getWidth(), ili9341_getHeight(), 0);
}

/* Text API mirrors original logic with PubSansBlack font */
int ili9341_putChar(char c, uint16_t x, uint16_t y, uint16_t fg, uint16_t bg) {
    if (c < 32 || c > 0x7E) return -1;
    uint8_t index = (uint8_t)(c - 32);
    uint16_t glyphI = PubSansBlack_glyph_dsc[index].glyph_index;
    uint8_t trueWid = PubSansBlack_glyph_dsc[index].w_px;
    if ((x + trueWid) > ili9341_getWidth()) return -1;
    if ((y + 32) > ili9341_getHeight()) return -1;

    uint16_t wid = ((trueWid % 8) == 0) ? trueWid : (uint16_t)(8 * (trueWid / 8 + 1));
    draw_intDrawBitmap1(x, y, trueWid, wid, 32, &PubSansBlack_glyph_bitmap[glyphI], fg, bg);
    return (int)trueWid + 1;
} /* [attached_file:3][attached_file:4] */

int ili9341_putStr(const char* s, uint16_t x, uint16_t y, uint16_t fg, uint16_t bg) {
    if (!s) return -1;
    int len = (int)strlen(s);
    uint16_t curx = x;
    for (int i = 0; i < len; ++i) {
        int adv = ili9341_putChar(s[i], curx, y, fg, bg);
        if (adv < 0) return adv;
        curx = (uint16_t)(curx + adv);
    }
    return (int)curx;
} /* [attached_file:3][attached_file:4] */

void ili9341_test(void) {
    ili9341_clear();
    int w = ili9341_getWidth();
    int h = ili9341_getHeight();

    //Top Left
    ili9341_drawRect(0, 0, 50, 50, ili9341_RGBto16bit(0, 0xFF, 0));

    //Bottom Left
    ili9341_drawRect(0, h - 50, 50, 50, ili9341_RGBto16bit(0, 0, 0xFF));

    //Top right
    ili9341_drawRect(w - 50, 0, 50, 50, ili9341_RGBto16bit(0xFF, 0, 0));

    //Bottom right
    ili9341_drawRect(w - 50, h - 50, 50, 50, ili9341_RGBto16bit(0x84, 0x45, 0x13));

    int bar = (w - 100) / 4;
    for (int y = 0; y < h; y++) {
        float p = (float)y / (float)h;
        uint8_t c = (uint8_t)(p * 255.0f);
        ili9341_drawRect(50, y, bar, 1, ili9341_RGBto16bit(c, 0, 0));
        ili9341_drawRect(50 + bar, y, bar, 1, ili9341_RGBto16bit(0, c, 0));
        ili9341_drawRect(50 + 2 * bar, y, bar, 1, ili9341_RGBto16bit(0, 0, c));

        ili9341_drawRect(50 + 3 * bar, y, bar, 1, ili9341_RGBto16bit(c, c, c));
    }
}
