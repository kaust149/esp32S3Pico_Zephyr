/*
 * PicoCalc LCD layer for the Epsilon (NumWorks) port.
 *
 * Presents the 320x240 RGB565 screen that Epsilon's Ion expects on top of the
 * PicoCalc's 320x320 ILI9488 panel. The three entry points match what Ion's
 * display backend needs (pushRect, pushRectUniform, pullRect). All coordinates
 * are logical: (0, 0) is the top-left of Epsilon's screen. Pixels are RGB565 in
 * native byte order, exactly like Epsilon's KDColor.
 *
 * A shadow copy of the screen is kept in PSRAM (when enabled) so pullRect can
 * read pixels back, which the write-only panel cannot do.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>

#include <zephyr/device.h>

#define LCD_WIDTH  320 /* Epsilon screen */
#define LCD_HEIGHT 240

/* The panel shows red and blue swapped: Zephyr's ILI9xxx driver always sets the BGR bit
 * in MADCTL, but this panel is RGB. Swapping in software avoids patching the driver. */
#define LCD_SWAP_RB 1

/* Takes the display device, clears the whole panel to black and the shadow too. */
int lcd_init(const struct device *display);

/* Draw w*h pixels (row-major, no padding) at logical (x, y). Clipped to the screen. */
void lcd_push_rect(int x, int y, int w, int h, const uint16_t *pixels);

/* Fill a rectangle with one colour. Clipped to the screen. */
void lcd_push_uniform(int x, int y, int w, int h, uint16_t color);

/* Read back w*h pixels (row-major) from the shadow copy. Out-of-screen parts read as 0. */
void lcd_pull_rect(int x, int y, int w, int h, uint16_t *pixels);
