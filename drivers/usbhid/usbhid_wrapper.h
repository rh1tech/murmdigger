/*
 * usbhid_wrapper.h - USB HID Keyboard Wrapper for Digger
 *
 * Copyright (c) 2026 Mikhail Matveev <xtreme@rh1.tech>
 * https://rh1.tech
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef USBHID_WRAPPER_H
#define USBHID_WRAPPER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef USB_HID_ENABLED

void usbhid_wrapper_init(void);
void usbhid_wrapper_tick(void);
int usbhid_wrapper_get_key(int *pressed, unsigned char *key);
bool usbhid_wrapper_is_key_pressed(uint8_t hid_code);

#else

static inline void usbhid_wrapper_init(void) {}
static inline void usbhid_wrapper_tick(void) {}
static inline int usbhid_wrapper_get_key(int *pressed, unsigned char *key) { (void)pressed; (void)key; return 0; }
static inline bool usbhid_wrapper_is_key_pressed(uint8_t hid_code) { (void)hid_code; return false; }

#endif

#ifdef __cplusplus
}
#endif

#endif /* USBHID_WRAPPER_H */
