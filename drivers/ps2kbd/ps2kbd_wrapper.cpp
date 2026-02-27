/*
 * ps2kbd_wrapper.cpp - PS/2 Keyboard wrapper for Digger Remastered
 * Provides raw HID keycode state tracking for GetAsyncKeyState().
 */

#include "../../src/board_config.h"
#include "ps2kbd_wrapper.h"
#include "ps2kbd_mrmltr.h"
#include <queue>
#include <string.h>

struct KeyEvent {
    int pressed;
    unsigned char key;  // HID keycode
};

static std::queue<KeyEvent> event_queue;

bool turbo_latched = false;
bool turbo_momentary = false;
bool show_speed = false;

// Raw HID key state: true = pressed
static bool hid_key_state[256];

static uint8_t current_modifiers = 0;

// Track arrow key state as bitfield (bits: 0=right, 1=left, 2=down, 3=up)
static uint8_t arrow_key_state = 0;

bool __not_in_flash() ps2kbd_is_turbo(void) {
    return turbo_latched || turbo_momentary;
}

bool __not_in_flash() ps2kbd_is_show_speed(void) {
    return show_speed;
}

uint32_t __not_in_flash() ps2kbd_get_numpad_state(void) {
    return 0;
}

static void key_handler(hid_keyboard_report_t *curr, hid_keyboard_report_t *prev) {
    current_modifiers = curr->modifier;

    // Update arrow key state
    arrow_key_state = 0;
    for (int i = 0; i < 6; i++) {
        uint8_t kc = curr->keycode[i];
        if (!kc) continue;
        if (kc == 0x4F) arrow_key_state |= 0x01;  // Right
        if (kc == 0x50) arrow_key_state |= 0x02;  // Left
        if (kc == 0x51) arrow_key_state |= 0x04;  // Down
        if (kc == 0x52) arrow_key_state |= 0x08;  // Up
    }

    // Clear all key states, then set from current report
    memset(hid_key_state, 0, sizeof(hid_key_state));
    for (int i = 0; i < 6; i++) {
        if (curr->keycode[i] != 0) {
            hid_key_state[curr->keycode[i]] = true;
        }
    }

    // Check for key presses (in curr but not in prev)
    for (int i = 0; i < 6; i++) {
        if (curr->keycode[i] != 0) {
            bool found = false;
            for (int j = 0; j < 6; j++) {
                if (prev->keycode[j] == curr->keycode[i]) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                // New key press - queue HID keycode
                event_queue.push({1, curr->keycode[i]});
            }
        }
    }

    // Check for key releases (in prev but not in curr)
    for (int i = 0; i < 6; i++) {
        if (prev->keycode[i] != 0) {
            bool found = false;
            for (int j = 0; j < 6; j++) {
                if (curr->keycode[j] == prev->keycode[i]) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                // Key released - queue HID keycode
                event_queue.push({0, prev->keycode[i]});
            }
        }
    }
}

static Ps2Kbd_Mrmltr* kbd = nullptr;

void ps2kbd_init(void) {
    memset(hid_key_state, 0, sizeof(hid_key_state));
    static Ps2Kbd_Mrmltr kbd_instance(pio0, PS2_PIN_CLK, key_handler);
    kbd = &kbd_instance;
    kbd->init_gpio();
}

void ps2kbd_tick(void) {
    if (kbd) {
        kbd->tick();
    }
}

int ps2kbd_get_key(int* pressed, unsigned char* key) {
    if (event_queue.empty()) {
        return 0;
    }
    KeyEvent ev = event_queue.front();
    event_queue.pop();
    *pressed = ev.pressed;
    *key = ev.key;
    return 1;
}

uint8_t ps2kbd_get_modifiers(void) {
    return current_modifiers;
}

uint8_t ps2kbd_get_arrow_state(void) {
    return arrow_key_state;
}

bool ps2kbd_is_reset_combo(void) {
    return false;
}

bool ps2kbd_is_key_pressed(uint8_t hid_code) {
    return hid_key_state[hid_code];
}
