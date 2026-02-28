/*
 * rp2350_main.c - RP2350 Entry Point for Digger
 *
 * Copyright (c) 2026 Mikhail Matveev <xtreme@rh1.tech>
 * https://rh1.tech
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/vreg.h"
#include "hardware/clocks.h"

#include "def.h"
#include "hardware.h"
#include "draw_api.h"
#include "sound.h"
#include "input.h"
#include "main.h"
#include "game.h"
#include "newsnd.h"
#include "board_config.h"
#include "HDMI.h"

/* Digger log file (redirect to NULL on RP2350) */
FILE *digger_log = NULL;

/*
 * Core 1 entry: HDMI video output.
 * Initializes HDMI IRQ handler on this core, then loops forever.
 * The HDMI driver runs entirely from DMA interrupts.
 */
static void __not_in_flash_func(core1_main)(void) {
    graphics_init_irq_on_this_core();

    /* Signal Core 0 that HDMI IRQ handler is ready */
    multicore_fifo_push_blocking(1);

    /* Loop forever in RAM. The HDMI DMA IRQ handler (__scratch_x) runs
     * on this core. Keeping core1_main in RAM ensures Core 1 never
     * accesses flash, so flash erase/program on Core 0 is safe without
     * multicore lockout - and HDMI signal stays uninterrupted. */
    while (true) {
        tight_loop_contents();
    }
}

/*
 * The CGA draw API is set up in sprite.c via dda_static (under #ifdef _RP2350).
 * ddap already points to the correct CGA function table - no override needed.
 */

/*
 * Initialize default game settings (replaces INI file loading).
 */
static void inir_defaults(void) {
    /* Game defaults */
    dgstate.nplayers = 1;
    dgstate.diggers = 1;
    dgstate.curplayer = 0;
    dgstate.startlev = 1;
    dgstate.levfflag = false;
    dgstate.gauntlet = false;
    dgstate.gtime = 120;
    dgstate.timeout = false;
    dgstate.unlimlives = false;
    dgstate.ftime = 80000;  /* 80ms per frame = 12.5 Hz */
    dgstate.cgtime = 0;
    dgstate.randv = 0;

    /* Sound defaults */
    soundflag = true;
    musicflag = true;
    volume = 1;

    /* Set up new sound engine */
    setupsound = s1setupsound;
    killsound = s1killsound;
    soundoff = s1soundoff;
    setspkrt2 = s1setspkrt2;
    timer0 = s1timer0;
    timer2 = s1timer2;
    soundinitglob(512, 44100);
}

/*
 * Main entry point for RP2350.
 */
int main(void) {
    /* Set CPU voltage and clock speed */
    vreg_set_voltage(CPU_VOLTAGE);
    sleep_ms(10);
    set_sys_clock_khz(CPU_CLOCK_MHZ * 1000, true);
    sleep_ms(10);

    /* USB serial console */
    stdio_init_all();
    sleep_ms(5000);
    printf("murmdigger: starting\n");

    /* Initialize HDMI graphics (Core 0, defer IRQ to Core 1) */
    graphics_set_defer_irq_to_core1(true);
    graphics_init(g_out_HDMI);

    /* Launch Core 1 for HDMI IRQ handling */
    multicore_launch_core1(core1_main);
    multicore_fifo_pop_blocking();  /* Wait until Core 1 has set IRQ handler */

    /* Initialize game with defaults (no INI file) */
    inir_defaults();

    /* Run the game */
    maininit();
    mainprog();

    return 0;
}
