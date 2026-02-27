/*
 * murmdigger - Simple DMA-based I2S Audio Driver for RP2350
 * Based on murmgenesis audio driver
 *
 * Stripped down to just I2S core functions for Digger's software sound.
 */
#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <hardware/pio.h>
#include <hardware/clocks.h>
#include <hardware/dma.h>

// Audio sample rate for Digger (matches SDL default)
#define AUDIO_SAMPLE_RATE 44100

// Audio buffer size - enough for one game frame at 12.5 Hz
// 44100 / 12.5 = 3528 samples per frame, round up with headroom
#define AUDIO_BUFFER_SAMPLES 4096

// I2S configuration structure
typedef struct {
    uint32_t sample_freq;
    uint16_t channel_count;
    uint8_t  data_pin;
    uint8_t  clock_pin_base;
    PIO      pio;
    uint8_t  sm;
    uint8_t  dma_channel;
    uint16_t dma_trans_count;
    uint16_t *dma_buf;
    uint8_t  volume;  // 0 = max volume, higher = quieter (shift amount)
} i2s_config_t;

// Get default I2S configuration
i2s_config_t i2s_get_default_config(void);

// Initialize I2S with the given configuration
void i2s_init(i2s_config_t *config);

// Write samples to I2S via DMA (non-blocking after first call)
// samples: pointer to stereo samples (interleaved L/R as 32-bit words)
void i2s_dma_write(i2s_config_t *config, const int16_t *samples);

// Write a specific number of samples
void i2s_dma_write_count(i2s_config_t *config, const int16_t *samples, uint32_t sample_count);

// Adjust volume (0 = loudest, 16 = quietest)
void i2s_volume(i2s_config_t *config, uint8_t volume);
void i2s_increase_volume(i2s_config_t *config);
void i2s_decrease_volume(i2s_config_t *config);

#endif // AUDIO_H
