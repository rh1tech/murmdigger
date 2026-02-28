/*
 * debug_log.h - Debug Logging Macros
 *
 * Copyright (c) 2026 Mikhail Matveev <xtreme@rh1.tech>
 * https://rh1.tech
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <stdio.h>

#ifndef ENABLE_DEBUG_LOGS
#define ENABLE_DEBUG_LOGS 0
#endif

#if ENABLE_DEBUG_LOGS
#define MII_DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define MII_DEBUG_PRINTF(...) do { } while (0)
#endif
