/*
 * rp2350_kbd.h - RP2350 PS/2 Keyboard Definitions for Digger
 *
 * Provides key state macros (leftpressed, rightpressed, etc.)
 * using GetAsyncKeyState() with HID keycodes, matching the
 * interface provided by sdl_kbd.h and fbsd_kbd.h.
 */

#ifndef __RP2350_KBD_H
#define __RP2350_KBD_H

bool GetAsyncKeyState(int);

#define rightpressed  (GetAsyncKeyState(keycodes[0][0]))
#define uppressed     (GetAsyncKeyState(keycodes[1][0]))
#define leftpressed   (GetAsyncKeyState(keycodes[2][0]))
#define downpressed   (GetAsyncKeyState(keycodes[3][0]))
#define f1pressed     (GetAsyncKeyState(keycodes[4][0]))
#define right2pressed (GetAsyncKeyState(keycodes[5][0]))
#define up2pressed    (GetAsyncKeyState(keycodes[6][0]))
#define left2pressed  (GetAsyncKeyState(keycodes[7][0]))
#define down2pressed  (GetAsyncKeyState(keycodes[8][0]))
#define f12pressed    (GetAsyncKeyState(keycodes[9][0]))

#endif
