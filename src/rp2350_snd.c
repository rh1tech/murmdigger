/*
 * rp2350_snd.c - RP2350 I2S Sound Backend
 *
 * Copyright (c) 2026 Mikhail Matveev <xtreme@rh1.tech>
 * https://rh1.tech
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "def.h"
#include "device.h"
#include "hardware.h"
#include "newsnd.h"
#include "audio.h"
#include "board_config.h"

bool wave_device_available = false;

static i2s_config_t i2s_config;
static bool audio_initialized = false;
static bool audio_paused = false;

/* Audio buffer for one game frame.
 * At 44100 Hz and ~12.5 Hz game rate: 44100/12.5 = 3528 samples per frame.
 * We use stereo (L+R), so 3528*2 = 7056 int16_t values per frame.
 */
#define AUDIO_SAMPLES_PER_FRAME 3528
static int16_t audio_buf[AUDIO_SAMPLES_PER_FRAME * 2] __attribute__((aligned(4)));

/*
 * setsounddevice - Initialize I2S audio hardware.
 */
bool setsounddevice(uint16_t samprate, uint16_t bufsize) {
    i2s_config = i2s_get_default_config();
    i2s_config.sample_freq = samprate;
    i2s_config.dma_trans_count = AUDIO_SAMPLES_PER_FRAME;

    i2s_init(&i2s_config);
    audio_initialized = true;
    wave_device_available = true;

    return true;
}

/*
 * initsounddevice - Stub (init done in setsounddevice).
 */
bool initsounddevice(void) {
    return true;
}

/*
 * pausesounddevice - Enable/disable audio output.
 */
void pausesounddevice(bool p) {
    audio_paused = p;
}

/*
 * audio_fill_and_submit - Generate audio samples and submit to I2S.
 *
 * Called once per game frame from the main loop.
 * Generates AUDIO_SAMPLES_PER_FRAME mono samples via getsample(),
 * duplicates to stereo, and submits to I2S DMA.
 */
void audio_fill_and_submit(void) {
    if (!audio_initialized || audio_paused)
        return;

    for (int i = 0; i < AUDIO_SAMPLES_PER_FRAME; i++) {
        int16_t s = getsample();
        audio_buf[i * 2] = s;      /* Left */
        audio_buf[i * 2 + 1] = s;  /* Right (mono->stereo) */
    }

    i2s_dma_write_count(&i2s_config, audio_buf, AUDIO_SAMPLES_PER_FRAME);
}
