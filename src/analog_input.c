#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/i2c.h"
#include "fft_functions.h"


#define SI4703_ADDR 0x10
#define RF_I2C_SDIO 36
#define RF_I2C_SCLK 37
#define RF_RST 40
#define ADC_PIN 41


volatile bool dma_flag = false;
volatile bool ping_active = true;
volatile int16_t ping_buf[NFFT];
volatile int16_t pong_buf[NFFT];

int dma_chan;


// void run_fft_on_dma() {
//     // Copy volatile DMA buffer to local array
//     uint16_t adc_local[NFFT];
//     // float f = 50.0f;
//     // for (int i = 0; i < NFFT; i++) {
//     //     float t = (float)i / SFREQ;
//     //     float sample = sinf(2.0f * M_PI * f * t);
//     //     adc_local[i] = (uint16_t)(2048 + 2047 * sample);  // 0–4095 range
//     // }
//     for (int i = 0; i < NFFT; i++) {
//         adc_local[i] = adc_buffer[i];
//     }
//     // for(int i=0;i<NFFT;i++){
//     //     printf("%u\n" ,adc_local[i]);
//     // }
//     // Call FFT on local copy
//     fft(adc_local);

//     dma_channel_set_read_addr(dma_chan, &adc_hw->fifo, false);
//     dma_channel_set_write_addr(dma_chan, adc_buffer, false);
//     dma_channel_set_transfer_count(dma_chan, NFFT, true);
// }

void dma_handler(){
    dma_hw->intr = 1u << dma_chan; // Clear the interrupt request
    dma_flag = true;
    if (ping_active) {
        //process_buffer(ping_buf);
        dma_channel_set_write_addr(dma_chan, pong_buf, true);
    } else {
        //process_buffer(pong_buf);
        dma_channel_set_write_addr(dma_chan, ping_buf, true);
    }
    ping_active = !ping_active;
}

void init_adc() {
  adc_init();
  adc_gpio_init(ADC_PIN);
  adc_select_input(1);

  adc_set_clkdiv(3200);

  adc_fifo_setup(
      true,
      true,
      1,
      false,
      false
  );
  adc_run(true);
}

void init_dma() {
    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(dma_chan);

    channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);
    channel_config_set_dreq(&c, DREQ_ADC);

    dma_channel_configure(
        dma_chan,
        &c,
        ping_buf,       // initial destination
        &adc_hw->fifo,  // source
        NFFT,       // number of transfers
        false           // don’t start yet
    );

    dma_channel_set_irq0_enabled(dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    dma_start_channel_mask(1u << dma_chan);
}

void rf_init_i2c() {
    i2c_init(i2c0, 400 * 1000);

    gpio_set_function(RF_I2C_SDIO, GPIO_FUNC_I2C); // SDA
    gpio_set_function(RF_I2C_SCLK, GPIO_FUNC_I2C); // SCL

    gpio_pull_up(RF_I2C_SDIO);
    gpio_pull_up(RF_I2C_SCLK);

    printf("I2C Init\n");
}

void rf_write_register(uint16_t reg02, uint16_t reg03, uint16_t reg04,
                        uint16_t reg05, uint16_t reg06, uint16_t reg07) {
    uint8_t buffer[12];

    // Register order: 0x02 -> 0x07
    buffer[0]  = (reg02 >> 8) & 0xFF;  // 0x02 Upper
    buffer[1]  = reg02 & 0xFF;         // 0x02 Lower
    buffer[2]  = (reg03 >> 8) & 0xFF;  // 0x03 Upper
    buffer[3]  = reg03 & 0xFF;         // 0x03 Lower
    buffer[4]  = (reg04 >> 8) & 0xFF;  // 0x04 Upper
    buffer[5]  = reg04 & 0xFF;         // 0x04 Lower
    buffer[6]  = (reg05 >> 8) & 0xFF;  // 0x05 Upper
    buffer[7]  = reg05 & 0xFF;         // 0x05 Lower
    buffer[8]  = (reg06 >> 8) & 0xFF;  // 0x06 Upper
    buffer[9]  = reg06 & 0xFF;         // 0x06 Lower
    buffer[10] = (reg07 >> 8) & 0xFF;  // 0x07 Upper
    buffer[11] = reg07 & 0xFF;         // 0x07 Lower


    int ret = i2c_write_blocking(i2c0, SI4703_ADDR, buffer, 12, true);
    printf("Write returned: %d\n", ret);
}


void rf_init() {

  //RST Toggled for 2 wire mode
  gpio_init(RF_RST);
  gpio_set_dir(RF_RST, true);
  gpio_put(RF_RST, false);
  sleep_ms(20);
  gpio_put(RF_RST, true);
  sleep_ms(20);

    uint16_t reg02 = 0;
    uint16_t reg03 = 0;
    uint16_t reg04 = 0;
    uint16_t reg05 = 0;
    uint16_t reg06 = 0;
    uint16_t reg07 = (1 << 15) | (1<<8);   // XOSCEN = 1

    rf_write_register(0,0,0,0,0,reg07);

    sleep_ms(500); // allow crystal to stabilize
    printf("XOSC enabled.\n");

    reg02 |= 0x4001;
    reg04 |= 1 << 12;
    reg05 |= (15 & 0x0F);  // Set volume to 15
    rf_write_register(reg02, reg03, reg04, reg05, reg06, reg07);
    sleep_ms(110);
}

void rf_read_register(uint16_t *regs) {
    uint8_t buffer[32];

    // Read 32 bytes directly (Si4703 auto-starts from 0x0A upper byte)
    int ret = i2c_read_blocking(i2c0, SI4703_ADDR, buffer, 32, false);
    if (ret < 0) {
        printf("I2C read failed\n");
        return;
    }

    // Convert to 16-bit register values and reorder them
    for (int i = 0; i < 16; i++) {
        int reg_index = (i + 10) % 16;
        regs[reg_index] = ((uint16_t)buffer[i * 2] << 8) | buffer[i * 2 + 1];
    }
}

void rf_print_registers(uint16_t *regs) {
    printf("SI4703 Register Dump:\n");
    printf("====================\n");
    for (int i = 0; i < 16; i++) {
        printf("Reg 0x%02X: 0x%04X\n", i, regs[i]);
    }
    printf("\n");
}

void rf_tune(uint16_t frequency_10x) {
    uint16_t regs[16];

    rf_read_register(regs); //reads current register values into regs

    // Convert frequency (in MHz * 10) to channel number
    // Formula: Channel = (Freq_MHz - 87.5) / 0.2
    // For integer math: Channel = (Freq_MHz * 10 - 875) / 2
    uint16_t channel = (frequency_10x - 875) / 2;

    printf("Tuning to: %u.%u MHz (Channel: %u)\n",
           frequency_10x / 10, frequency_10x % 10, channel);

    // Set TUNE bit high and set CHAN[9:0] bits in register 0x03
    regs[0x03] = 0x8000 | (channel & 0x03FF);  // TUNE = 1, CHAN[9:0] = channel

    printf("Writing to reg 0x03: 0x%04X\n", regs[0x03]);

    // Write to start tune operation
    rf_write_register(regs[0x02], regs[0x03], regs[0x04],
                     regs[0x05], regs[0x06], regs[0x07]);

    printf("Tune operation started...\n");

    // Wait for completion (polling STC bit)
    int timeout = 0;
    do {
        sleep_ms(50);
        rf_read_register(regs);
        timeout++;

        if (timeout > 20) { // 1 second timeout
            printf("Tune timeout!\n");
            break;
        }
    } while (!(regs[0x0A] & (1 << 14))); // Wait for STC bit

    // Read status after tuning
    rf_read_register(regs);

    // Check tune results
    if (regs[0x0A] & (1 << 14)) {  // STC bit set
        printf("Tuning completed!\n");

        // Extract current channel from register 0x0B (optional)
        uint16_t current_channel = regs[0x0B] & 0x03FF;  // READCHAN[9:0]
        uint16_t actual_frequency = (current_channel * 2 + 875);

        printf("Tuned to: %u.%u MHz (Channel: %u)\n",
               actual_frequency / 10, actual_frequency % 10, current_channel);

        // Check stereo and signal strength
        if (regs[0x0A] & (1 << 8)) {  // ST bit
            printf("Stereo signal detected\n");
        } else {
            printf("Mono signal\n");
        }

        uint8_t rssi = regs[0x0A] & 0xFF;  // RSSI[7:0]
        printf("Signal strength (RSSI): %u\n", rssi);
    }

    // Clear TUNE bit to end tuning operation
    regs[0x03] &= ~0x8000;  // TUNE = 0, keep channel bits
    rf_write_register(regs[0x02], regs[0x03], regs[0x04],
                     regs[0x05], regs[0x06], regs[0x07]);
}

void rf_set_volume(uint16_t volume) {
    uint16_t regs[16];
    rf_read_register(regs);
    regs[0x05] = (regs[0x05] & 0xFFF0) | (volume & 0x0F); // Set volume bits
    rf_write_register(regs[0x02], regs[0x03], regs[0x04],
                     regs[0x05], regs[0x06], regs[0x07]);
    printf("Volume set to %u\n", volume);
}