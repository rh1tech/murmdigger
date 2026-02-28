/*
 * usbhid_wrapper.c - USB HID Keyboard Wrapper for Digger
 *
 * Bridges the generic USB HID driver (usbhid.h) to Digger's keyboard
 * interface. Tracks per-key state and provides a ring buffer of
 * press/release events in the same format as ps2kbd_wrapper.
 *
 * Copyright (c) 2026 Mikhail Matveev <xtreme@rh1.tech>
 * https://rh1.tech
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifdef USB_HID_ENABLED

#include "usbhid_wrapper.h"
#include "usbhid.h"
#include <string.h>

/* Per-key held state (indexed by HID keycode 0-255) */
static bool usb_key_state[256];

/* Ring buffer for press/release events */
#define USB_EVENT_BUF_SIZE 32

typedef struct {
    int pressed;
    unsigned char key;
} usb_key_event_t;

static usb_key_event_t usb_event_buf[USB_EVENT_BUF_SIZE];
static int usb_event_head = 0;
static int usb_event_tail = 0;

static void push_event(int pressed, unsigned char key) {
    int next = (usb_event_head + 1) % USB_EVENT_BUF_SIZE;
    if (next != usb_event_tail) {
        usb_event_buf[usb_event_head].pressed = pressed;
        usb_event_buf[usb_event_head].key = key;
        usb_event_head = next;
    }
}

void usbhid_wrapper_init(void) {
    memset(usb_key_state, 0, sizeof(usb_key_state));
    usb_event_head = 0;
    usb_event_tail = 0;
    usbhid_init();
}

void usbhid_wrapper_tick(void) {
    usbhid_task();

    uint8_t keycode;
    int down;
    while (usbhid_get_key_action(&keycode, &down)) {
        usb_key_state[keycode] = down ? true : false;
        push_event(down, keycode);
    }
}

int usbhid_wrapper_get_key(int *pressed, unsigned char *key) {
    if (usb_event_head == usb_event_tail)
        return 0;

    *pressed = usb_event_buf[usb_event_tail].pressed;
    *key = usb_event_buf[usb_event_tail].key;
    usb_event_tail = (usb_event_tail + 1) % USB_EVENT_BUF_SIZE;
    return 1;
}

bool usbhid_wrapper_is_key_pressed(uint8_t hid_code) {
    return usb_key_state[hid_code];
}

#endif /* USB_HID_ENABLED */
