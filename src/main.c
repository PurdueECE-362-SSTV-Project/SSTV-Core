#include <stdio.h>
#include <string.h>

// Pico Libraries
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "pico/multicore.h"

// Custom
#include "I_Q_analysis.h"
#include "input_control.h"
#include "Ili9341.h"
#include "ili9341hw.h"
#include "fft_functions.h"
#include "decoder.h"

uint32_t adc_fifo_out = 0;
extern volatile bool dma_flag;
extern volatile bool ping_active;
extern volatile uint16_t ping_buf[NFFT];
extern volatile uint16_t pong_buf[NFFT];

extern volatile bool rotary_isr_flag;
extern volatile bool rotary_switch_flag;
extern volatile bool frontbutton1_flag;
extern volatile bool frontbutton2_flag;
extern volatile bool frontbutton3_flag;

extern volatile uint16_t frequency10x;
extern volatile uint16_t volume_level;
extern volatile uint16_t freq_flag;

#define DC_OFFSET 2048

//////////////////////////////////////////////////////////////////////////////

void core1_main() {
    sleep_ms(10);
    rf_init_i2c();
    rf_init();
    uint16_t regs[16];

    rf_read_register(regs);
    rf_print_registers(regs);

    init_rotary_encoder();
    init_rotary_irq();

    // Initialize the imageBuffer
    init_imageBuffer();

    // Init the TFT display
    ili9341_init(spi0,
                60 * MHz,
                PIN_SDO,		//SDO(MISO)
                PIN_CS, 		//CS
                PIN_SCK,		//SCK
                PIN_SDI,		//SDI(MOSI)
                PIN_nRESET,  	//RESET
                PIN_DC,			//DC/RS
                PIN_LED,
                true);

    display_init_dma();

    sleep_ms(500);      // Wait for stdio to initialize

    printf("Core 1 initialized and entering main loop.\n");

    for (;;) {
        if(rotary_isr_flag){
            rotary_isr_flag = false;
            rotary_logic(freq_flag);
        }
        else if(rotary_switch_flag){
            //update frequency and volume
            rotary_switch_flag = false;
            rotary_switch_logic(frequency10x, volume_level);
        }
        else if (frontbutton1_flag){
            frontbutton1_flag = false;
            printf("Freq_flag = %d\n", freq_flag);
        }
        else if (frontbutton2_flag){
            frontbutton2_flag = false;
            printf("Front Button 2 Pressed\n");
        }
        else if (frontbutton3_flag){
            frontbutton3_flag = false;
            printf("Front Button 3 Pressed\n");
        }
    }
}


//////////////////////////////////////////////////////////////////////////////

int main()
{
    stdio_init_all();
    sleep_ms(1000);
    multicore_launch_core1(core1_main);
    sleep_ms(1000); // Wait for stdio to initialize

    init_dma();

    init_adc();
    init_adc_dma();

    cordic_init();
    filter_init(&ssb_filter);

    // Write Test Colours on Screen
    ili9341_test();

    sleep_ms(3000);

    //ili9341_clear();
    // OR
	ILI9341_setScreenColour(TFT_BLACK);

    // Draw A Small Rectangle
    ili9341_drawRect(120, 100, 20, 20, TFT_AQUA);
    sleep_ms(100);

    {
        ili9341_putStr("Hello", 20, 20, TFT_AZURE, TFT_LIGHTSEAGREEN);
    }

    uint16_t counter = 0;
    for (;;){
        if(dma_flag) {
            dma_flag = false;
            //run_fft_on_dma();
            int16_t freq_sample = 0;
            int16_t current_sample = 0;
            for(int i=0;i<NFFT;i++){
                if(ping_active) {
                    current_sample = ping_buf[i];
                } else {
                    current_sample = pong_buf[i];
                }
                current_sample -= DC_OFFSET;            // DC offset
                decode_audio(current_sample, &freq_sample);
                counter++;
                decoder(freq_sample);
                if (counter == 1024) {
                    counter = 0;
                    //printf("Freq sample: %d\n", freq_sample);
                }
            }

        }
        tight_loop_contents();
    }
    return 0;

}
