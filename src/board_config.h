/*
 * board_config.h - Board GPIO Configuration
 *
 * Copyright (c) 2026 Mikhail Matveev <xtreme@rh1.tech>
 * https://rh1.tech
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include "pico.h"
#include "hardware/vreg.h"

/*
 * Board Configuration Variants:
 *
 * BOARD_M1 - M1 GPIO layout
 * BOARD_M2 - M2 GPIO layout
 */

// Default to M1 if no config specified
#if !defined(BOARD_M1) && !defined(BOARD_M2)
#define BOARD_M1
#endif

//=============================================================================
// CPU Speed Defaults (can be overridden via CMake)
//=============================================================================
#ifndef CPU_CLOCK_MHZ
#define CPU_CLOCK_MHZ 252
#endif

#ifndef CPU_VOLTAGE
#define CPU_VOLTAGE VREG_VOLTAGE_1_50
#endif

//=============================================================================
// M1 Layout Configuration
//=============================================================================
#ifdef BOARD_M1

// HDMI Pins
#define HDMI_PIN_CLKN 6
#define HDMI_PIN_CLKP 7
#define HDMI_PIN_D0N  8
#define HDMI_PIN_D0P  9
#define HDMI_PIN_D1N  10
#define HDMI_PIN_D1P  11
#define HDMI_PIN_D2N  12
#define HDMI_PIN_D2P  13

#define HDMI_BASE_PIN HDMI_PIN_CLKN

// PS/2 Keyboard Pins
#define PS2_PIN_CLK  0
#define PS2_PIN_DATA 1

// I2S Audio Pins
#define I2S_DATA_PIN       26
#define I2S_CLOCK_PIN_BASE 27

#endif // BOARD_M1

//=============================================================================
// M2 Layout Configuration
//=============================================================================
#ifdef BOARD_M2

// HDMI Pins
#define HDMI_PIN_CLKN 12
#define HDMI_PIN_CLKP 13
#define HDMI_PIN_D0N  14
#define HDMI_PIN_D0P  15
#define HDMI_PIN_D1N  16
#define HDMI_PIN_D1P  17
#define HDMI_PIN_D2N  18
#define HDMI_PIN_D2P  19

#define HDMI_BASE_PIN HDMI_PIN_CLKN

// PS/2 Keyboard Pins
#define PS2_PIN_CLK  2
#define PS2_PIN_DATA 3

// I2S Audio Pins
#define I2S_DATA_PIN       9
#define I2S_CLOCK_PIN_BASE 10

#endif // BOARD_M2

//=============================================================================
// Digger Display Configuration
//=============================================================================
#define DIGGER_WIDTH  320
#define DIGGER_HEIGHT 200

// HDMI framebuffer is 320x240, Digger uses 320x200
// Center vertically with 20-pixel offset
#define DIGGER_Y_OFFSET 20

// HDMI output resolution (provided by HDMI driver)
#define HDMI_WIDTH  320
#define HDMI_HEIGHT 240

#endif // BOARD_CONFIG_H
