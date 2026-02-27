/*
 * rp2350_kbd.c - RP2350 PS/2 Keyboard Backend for Digger
 *
 * Uses PS/2 keyboard driver with HID keycodes.
 * Maps Digger key functions to HID scancodes.
 */

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "def.h"
#include "hardware.h"
#include "input.h"
#include "ps2kbd/ps2kbd_wrapper.h"
#include "ps2kbd/hid_codes.h"

#define KBLEN 30

struct kbent {
    int16_t scancode;
};

static struct kbent kbuffer[KBLEN];
static int16_t klen = 0;

/*
 * Key mappings using HID keycodes.
 * keycodes[NKEYS][5]: up to 5 alternative keys per function.
 * -2 = unused slot.
 */
int keycodes[NKEYS][5] = {
    {HID_KEY_ARROW_RIGHT, -2, -2, -2, -2},  /* P1 Right */
    {HID_KEY_ARROW_UP,    -2, -2, -2, -2},  /* P1 Up */
    {HID_KEY_ARROW_LEFT,  -2, -2, -2, -2},  /* P1 Left */
    {HID_KEY_ARROW_DOWN,  -2, -2, -2, -2},  /* P1 Down */
    {HID_KEY_F1,          -2, -2, -2, -2},  /* P1 Fire */
    {HID_KEY_S,           -2, -2, -2, -2},  /* P2 Right */
    {HID_KEY_W,           -2, -2, -2, -2},  /* P2 Up */
    {HID_KEY_A,           -2, -2, -2, -2},  /* P2 Left */
    {HID_KEY_Z,           -2, -2, -2, -2},  /* P2 Down */
    {HID_KEY_TAB,         -2, -2, -2, -2},  /* P2 Fire */
    {HID_KEY_T,           -2, -2, -2, -2},  /* Cheat */
    {HID_KEY_KEYPAD_ADD,  -2, -2, -2, -2},  /* Accelerate */
    {HID_KEY_KEYPAD_SUBTRACT, -2, -2, -2, -2},  /* Brake */
    {HID_KEY_F7,          -2, -2, -2, -2},  /* Music toggle */
    {HID_KEY_F9,          -2, -2, -2, -2},  /* Sound toggle */
    {HID_KEY_F10,         -2, -2, -2, -2},  /* Exit */
    {HID_KEY_SPACE,       -2, -2, -2, -2},  /* Pause */
    {HID_KEY_N,           -2, -2, -2, -2},  /* Change mode */
    {HID_KEY_F8,          -2, -2, -2, -2},  /* Save DRF */
};

/*
 * Poll PS/2 keyboard and drain events into our buffer.
 */
static void poll_keyboard(void) {
    int pressed;
    unsigned char key;

    ps2kbd_tick();

    while (ps2kbd_get_key(&pressed, &key)) {
        if (pressed && klen < KBLEN) {
            kbuffer[klen].scancode = key;
            klen++;
        }
    }
}

/*
 * GetAsyncKeyState - Check if a specific HID key is currently held.
 */
bool GetAsyncKeyState(int key) {
    ps2kbd_tick();
    return ps2kbd_is_key_pressed((uint8_t)key);
}

/*
 * initkeyb - Initialize PS/2 keyboard driver.
 */
void initkeyb(void) {
    ps2kbd_init();
    klen = 0;
}

/*
 * restorekeyb - No-op on RP2350.
 */
void restorekeyb(void) {
}

/*
 * hid_to_ascii - Convert HID keycode to ASCII character.
 * Returns 0 for unmapped keys.
 */
static int16_t hid_to_ascii(int16_t hid) {
    if (hid >= HID_KEY_A && hid <= HID_KEY_Z)
        return 'A' + (hid - HID_KEY_A);
    if (hid >= HID_KEY_1 && hid <= HID_KEY_9)
        return '1' + (hid - HID_KEY_1);
    if (hid == HID_KEY_0)
        return '0';
    if (hid == HID_KEY_ENTER)
        return 13;
    if (hid == HID_KEY_BACKSPACE)
        return 8;
    if (hid == HID_KEY_DELETE)
        return 127;
    if (hid == HID_KEY_SPACE)
        return ' ';
    return 0;
}

/*
 * getkey - Block until a key is pressed.
 * If scancode=true, return raw HID scancode (for game controls).
 * If scancode=false, return ASCII character (for text input like initials).
 */
int16_t getkey(bool scancode) {
    int16_t result;

    while (!kbhit())
        gethrt(true);

    result = kbuffer[0].scancode;
    klen--;
    if (klen > 0)
        memmove(kbuffer, kbuffer + 1, klen * sizeof(struct kbent));

    if (!scancode)
        result = hid_to_ascii(result);

    return result;
}

/*
 * kbhit - Check if any key event is available.
 */
bool kbhit(void) {
    poll_keyboard();
    return klen > 0;
}
