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

// Lookup table for reversing bits in a byte (0 - 255)
static const uint8_t bit_reverse_table[256] = {
    0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
    0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
    0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
    0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
    0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
    0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
    0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
    0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
    0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
    0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
    0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
    0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
    0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
    0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
    0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
    0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
    0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
    0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
    0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
    0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
    0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
    0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
    0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
    0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
    0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
    0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
    0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
    0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
    0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
    0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
    0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
    0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF,
};

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

// Function to reverse bits of a 16-bit number using the lookup table
uint16_t reverse_bits_16_lut(uint16_t x) {
    return (bit_reverse_table[x & 0xFF] << 8) | bit_reverse_table[(x >> 8) & 0xFF];
}

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
static void ILI9341_WriteData(const void* buf, int bytes) {
    ILI9341_CS_Set(CS_ENABLE);
    spi_write_blocking(s_spi, (const uint8_t*)buf, bytes);
    ILI9341_CS_Set(CS_DISABLE);
}

/// @brief Writes an image buffer to display
void ILI9341_writeImageBuffer(void) {
    if (!imageBuffer) return;
    ILI9341_SetOutWriting(0, PIX_WIDTH - 1, 0, PIX_HEIGHT - 1);
    for (int y = PIX_HEIGHT - 1; y >= 0; --y) {
        const uint16_t* row = (const uint16_t*)&imageBuffer[y * PIX_WIDTH];
        ILI9341_WriteData(row, (int)(PIX_WIDTH * sizeof(uint16_t)));
    }
}

/// @brief Set the entire Screen a colour
void ILI9341_setScreenColour(uint16_t color16) {
    if (!imageBuffer) return;
    color16 = reverse_bits_16_lut(color16);
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
        ILI9341_WriteData(data, 15);
    }

    // Negative gamma correction.
    ILI9341_SetCommand(ILI9341_GMCTRN1);
    {
        uint8_t data2[15] = {0x00,0x0e,0x14,0x03,0x11,0x07,0x31,0xc1,0x48,0x08,0x0f,0x0c,0x31,0x36,0x0f};
        ILI9341_WriteData(data2, 15);
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

    color = reverse_bits_16_lut(color);
    
    for (int idx_y = y; (idx_y < y + hei) && (idx_y < PIX_HEIGHT); ++idx_y) {
        for (int idx_x = x; (idx_x < x + wid) && (idx_x < PIX_WIDTH); ++idx_x) {
            imageBuffer[PIX(idx_x, idx_y)] = color;
        }
    }
}

void ili9341_drawPixel(uint16_t x, uint16_t y, uint16_t color) {
    if (!imageBuffer) return;
    
    imageBuffer[PIX(x, y)] = reverse_bits_16_lut(color);
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
