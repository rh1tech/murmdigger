/*
 * rp2350_vid.c - RP2350 HDMI Video Backend for Digger
 *
 * Renders CGA 4-color graphics to the HDMI 4-bit indexed framebuffer.
 * The HDMI driver provides a 320x240 framebuffer with 4 bits per pixel
 * (nibble-packed, 2 pixels per byte). Digger's 320x200 is rendered
 * with a 20-pixel Y offset for vertical centering.
 */

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "def.h"
#include "hardware.h"
#include "draw_api.h"
#include "alpha.h"
#include "board_config.h"
#include "HDMI.h"

/* CGA sprite table from cgagrafx.c */
extern const uint8_t *cgatable[];

/* CGA alpha font from alpha.c - 2bpp packed, 3 bytes/row, 12 rows */
extern const uint8_t * const ascii2cga[];

/*
 * CGA Palette definitions (RGB888)
 *
 * CGA Palette 0 (pal=0): Black, Green, Red, Brown
 * CGA Palette 1 (pal=1): Black, Cyan, Magenta, White
 */
static const uint32_t cga_pal0[] =  {0x000000, 0x00AA00, 0xAA0000, 0xAA5500};
static const uint32_t cga_pal0i[] = {0x000000, 0x55FF55, 0xFF5555, 0xFFFF55};
static const uint32_t cga_pal1[] =  {0x000000, 0x00AAAA, 0xAA00AA, 0xAAAAAA};
static const uint32_t cga_pal1i[] = {0x000000, 0x55FFFF, 0xFF55FF, 0xFFFFFF};

static int16_t current_pal = 0;
static int16_t current_inten = 0;

/* Pointer to HDMI framebuffer (4-bit nibble-packed, 320x240) */
static uint8_t *framebuffer;

/*
 * Framebuffer access helpers.
 * The HDMI framebuffer is 4-bit per pixel, nibble-packed.
 * HDMI DMA reads: low nibble = left pixel (even x), high nibble = right pixel (odd x).
 * Row stride = 320/2 = 160 bytes.
 */
#define FB_STRIDE (HDMI_WIDTH / 2)

static inline void fb_set_pixel(int x, int y, uint8_t color) {
    int fb_y = y + DIGGER_Y_OFFSET;
    if (fb_y < 0 || fb_y >= HDMI_HEIGHT || x < 0 || x >= HDMI_WIDTH)
        return;
    int idx = fb_y * FB_STRIDE + (x >> 1);
    if (x & 1)
        framebuffer[idx] = (framebuffer[idx] & 0x0F) | ((color & 0x0F) << 4);
    else
        framebuffer[idx] = (framebuffer[idx] & 0xF0) | (color & 0x0F);
}

static inline uint8_t fb_get_pixel(int x, int y) {
    int fb_y = y + DIGGER_Y_OFFSET;
    if (fb_y < 0 || fb_y >= HDMI_HEIGHT || x < 0 || x >= HDMI_WIDTH)
        return 0;
    int idx = fb_y * FB_STRIDE + (x >> 1);
    if (x & 1)
        return (framebuffer[idx] >> 4) & 0x0F;
    else
        return framebuffer[idx] & 0x0F;
}

/*
 * Apply CGA palette to HDMI palette entries 0-3.
 */
static void apply_palette(void) {
    const uint32_t *pal;
    if (current_pal == 0)
        pal = current_inten ? cga_pal0i : cga_pal0;
    else
        pal = current_inten ? cga_pal1i : cga_pal1;

    for (int i = 0; i < 4; i++)
        graphics_set_palette(i, pal[i]);
}

/*
 * rp2350_init - Initialize HDMI video
 */
void cgainit(void) {
    framebuffer = graphics_get_buffer();
    apply_palette();
}

/*
 * rp2350_clear - Clear entire framebuffer to black
 */
void cgaclear(void) {
    memset(framebuffer, 0, FB_STRIDE * HDMI_HEIGHT);
}

/*
 * rp2350_pal - Set CGA palette (0 or 1)
 */
void cgapal(int16_t pal) {
    current_pal = pal;
    apply_palette();
}

/*
 * rp2350_inten - Switch between normal/high intensity palette
 */
void cgainten(int16_t inten) {
    current_inten = inten;
    apply_palette();
}

/*
 * rp2350_puti - Copy raw 4-bit packed pixels from buffer p to framebuffer
 *
 * Buffer format: 4-bit nibble-packed, same as framebuffer.
 * w = width in "sprite units" (w*4 = pixel width), h = height in pixels.
 * Buffer stores (w*4/2) = w*2 bytes per row.
 */
void cgaputi(int16_t x, int16_t y, uint8_t *p, int16_t w, int16_t h) {
    int pixel_w = w * 4;  /* width in pixels */
    int buf_stride = pixel_w / 2;  /* bytes per row in buffer */

    for (int row = 0; row < h; row++) {
        int fb_y = (y + row) + DIGGER_Y_OFFSET;
        if (fb_y < 0 || fb_y >= HDMI_HEIGHT)
            continue;
        int fb_offset = fb_y * FB_STRIDE + (x >> 1);
        int buf_offset = row * buf_stride;
        memcpy(&framebuffer[fb_offset], &p[buf_offset], buf_stride);
    }
}

/*
 * rp2350_geti - Copy 4-bit packed pixels from framebuffer to buffer p
 */
void cgageti(int16_t x, int16_t y, uint8_t *p, int16_t w, int16_t h) {
    int pixel_w = w * 4;
    int buf_stride = pixel_w / 2;

    for (int row = 0; row < h; row++) {
        int fb_y = (y + row) + DIGGER_Y_OFFSET;
        if (fb_y < 0 || fb_y >= HDMI_HEIGHT)
            continue;
        int fb_offset = fb_y * FB_STRIDE + (x >> 1);
        int buf_offset = row * buf_stride;
        memcpy(&p[buf_offset], &framebuffer[fb_offset], buf_stride);
    }
}

/*
 * rp2350_putim - Draw CGA sprite with mask (transparency)
 *
 * CGA sprite data: 2 bits per pixel, 4 pixels per byte.
 * cgatable[ch*2] = sprite data, cgatable[ch*2+1] = mask.
 * w = bytes per row (each byte = 4 pixels), h = rows.
 *
 * For each pixel: result = (screen & mask) | sprite
 * Mask bit 1 = transparent (keep screen), mask bit 0 = opaque (use sprite)
 *
 * CGA 2-bit pixels map directly to palette indices 0-3.
 */
void cgaputim(int16_t x, int16_t y, int16_t ch, int16_t w, int16_t h) {
    const uint8_t *sprite = cgatable[ch * 2];
    const uint8_t *mask = cgatable[ch * 2 + 1];

    for (int row = 0; row < h; row++) {
        int px = x;
        for (int col = 0; col < w; col++) {
            uint8_t sbyte = sprite[row * w + col];
            uint8_t mbyte = mask[row * w + col];

            /* Each byte has 4 pixels at 2 bits each, MSB first */
            for (int bit = 6; bit >= 0; bit -= 2) {
                uint8_t spix = (sbyte >> bit) & 0x03;
                uint8_t mpix = (mbyte >> bit) & 0x03;

                if (mpix != 0x03) {
                    /* Not fully transparent - blend */
                    uint8_t screen_pix = fb_get_pixel(px, y + row);
                    uint8_t result = (screen_pix & mpix) | spix;
                    fb_set_pixel(px, y + row, result);
                } else if (spix != 0) {
                    /* Mask is fully transparent but sprite has data */
                    fb_set_pixel(px, y + row, spix);
                }
                px++;
            }
        }
    }
}

/*
 * rp2350_getpix - Read pixel collision data from framebuffer
 *
 * Returns CGA-format byte: 4 pixels at 2 bits each, MSB first.
 *   bits 7-6: color of pixel at (x, y)
 *   bits 5-4: color of pixel at (x+1, y)
 *   bits 3-2: color of pixel at (x+2, y)
 *   bits 1-0: color of pixel at (x+3, y)
 *
 * Single row only (matching original CGA video memory read).
 * Callers use bit masks to check specific pixel positions:
 *   0xc0 = leftmost pixel (x), 0x03 = rightmost pixel (x+3)
 */
int16_t cgagetpix(int16_t x, int16_t y) {
    int16_t rval = 0;

    if (x < 0 || x > 319 || y < 0 || y > 199)
        return 0xff;

    for (int xi = 0; xi < 4; xi++) {
        uint8_t pix = fb_get_pixel(x + xi, y) & 0x03;
        rval |= pix << (6 - xi * 2);
    }

    return rval;
}

/*
 * rp2350_write - Draw text character
 *
 * CGA alpha font: 3 bytes per row, 12 rows.
 * Each byte = 4 pixels at 2 bits each = 12 pixels wide.
 * Font uses values 0 (background) and 3 (foreground).
 * Parameter c = CGA palette index for foreground color.
 */
void cgawrite(int16_t x, int16_t y, int16_t ch, int16_t c) {
    const uint8_t *font;

    if (!isvalchar(ch))
        return;

    font = ascii2cga[ch - 32];
    if (font == NULL)
        return;

    for (int row = 0; row < 12; row++) {
        int px = x;
        for (int col = 0; col < 3; col++) {
            uint8_t byte = font[row * 3 + col];
            for (int bit = 6; bit >= 0; bit -= 2) {
                uint8_t pix = (byte >> bit) & 0x03;
                fb_set_pixel(px, y + row, pix ? c : 0);
                px++;
            }
        }
    }
}

/*
 * rp2350_title - Draw title screen
 *
 * Draws CGA-style red border and copyright text.
 * The game's mainprog() draws "D I G G E R", high scores,
 * and character animations on top.
 */
void cgatitle(void) {
    int x, y, t;

    cgaclear();

    /* Draw red border (color 2) with vertical divider.
     * 3 pixels thick, matching original CGA title screen. */
    #define BRD_L   4    /* left outer edge */
    #define BRD_R   317  /* right outer edge (past erasetext reach at x=314) */
    #define BRD_T   16   /* top outer edge */
    #define BRD_B   185  /* bottom outer edge */
    #define BRD_W   3    /* thickness */
    #define BRD_DIV 160  /* vertical divider x center */

    /* Top and bottom horizontal bars */
    for (x = BRD_L; x <= BRD_R; x++)
        for (t = 0; t < BRD_W; t++) {
            fb_set_pixel(x, BRD_T + t, 2);
            fb_set_pixel(x, BRD_B - t, 2);
        }
    /* Left and right vertical bars */
    for (y = BRD_T; y <= BRD_B; y++)
        for (t = 0; t < BRD_W; t++) {
            fb_set_pixel(BRD_L + t, y, 2);
            fb_set_pixel(BRD_R - t, y, 2);
        }
    /* Vertical divider */
    for (y = BRD_T; y <= BRD_B; y++)
        for (t = 0; t < BRD_W; t++)
            fb_set_pixel(BRD_DIV - 1 + t, y, 2);
}

/*
 * rp2350_flush - No-op (HDMI DMA auto-reads framebuffer)
 */
void doscreenupdate(void) {
    /* HDMI driver continuously DMA-reads the framebuffer */
}

/*
 * graphicsoff - No-op on RP2350
 */
void graphicsoff(void) {
}

/*
 * gretrace - No-op on RP2350
 */
void gretrace(void) {
}
