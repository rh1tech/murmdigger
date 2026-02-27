/*
 * rp2350_timer.c - RP2350 Hardware Timer Backend for Digger
 *
 * Uses RP2350's hardware timer (time_us_64()) for frame timing.
 * Replaces SDL_GetTicks() / SDL_Delay() from sdl_timer.c.
 */

#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"

#include "def.h"
#include "hardware.h"
#include "digger_math.h"
#include "game.h"

/* Audio fill from rp2350_snd.c - called each frame to generate samples */
extern void audio_fill_and_submit(void);

/* HDMI watchdog from HDMI.c - restarts DMA if stalled */
extern bool hdmi_check_and_restart(void);

/* Frame timing state */
static uint64_t next_frame_time_us = 0;
static bool timer_initialized = false;

/*
 * inittimer - Initialize frame timing.
 */
void inittimer(void) {
    next_frame_time_us = time_us_64() + dgstate.ftime;
    timer_initialized = true;
}

/*
 * gethrt - Frame synchronization.
 *
 * Waits until the next frame boundary, then advances the target time.
 * No need to call doscreenupdate() since HDMI DMA auto-refreshes.
 */
void gethrt(bool minsleep) {
    /* Pump audio each frame - generates samples and calls soundint() */
    audio_fill_and_submit();

    /* Check HDMI DMA health, restart if stalled */
    hdmi_check_and_restart();

    if (!timer_initialized || dgstate.ftime <= 1) {
        if (minsleep)
            sleep_us(10000);  /* 10ms minimum sleep */
        return;
    }

    uint64_t now = time_us_64();

    if (now < next_frame_time_us) {
        uint64_t delay = next_frame_time_us - now;
        if (delay > 200000)
            delay = 200000;  /* Cap at 200ms to prevent long stalls */
        sleep_us(delay);
    }

    next_frame_time_us += dgstate.ftime;

    /* If we fell behind, reset to now + ftime */
    now = time_us_64();
    if (next_frame_time_us < now)
        next_frame_time_us = now + dgstate.ftime;
}

/*
 * getkips - Return processor speed estimate.
 * Returns 1 (stub, same as SDL version).
 */
int32_t getkips(void) {
    return 1;
}

/*
 * olddelay - Delay in milliseconds.
 */
void olddelay(int16_t t) {
    if (t > 0)
        sleep_ms(t);
}

/* Sound hardware stubs (timer-based sound control not used on RP2350) */
void s0soundoff(void) {}
void s0setspkrt2(void) {}
void s0settimer0(uint16_t t0v) {}
void s0settimer2(uint16_t t0v, bool mode) {}
void s0timer0(uint16_t t0v) {}
void s0timer2(uint16_t t0v, bool mode) {}
void s0soundinitglob(void) {}
void s0soundkillglob(void) {}
