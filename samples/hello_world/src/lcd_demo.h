/*
 * On-board check of the LCD layer (lcd.h) with Epsilon-style RGB565 drawing.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <zephyr/device.h>

/*
 * Initialises the LCD layer, then draws on the 320x240 logical screen and prints
 * timings and a read-back check on the console:
 *   - four 60-row ramps, top to bottom: red, green, blue, grey (0 to 255 left to right);
 *   - a 1-pixel white frame around the screen;
 *   - 16x16 corner markers: red top-left, green top-right, blue bottom-left,
 *     white bottom-right;
 *   - a checker block and a uniform block in the middle, pushed as partial updates.
 * The 40 rows above and below the 240-row picture stay black.
 * Returns 0 when the read-back matches what was drawn, otherwise a negative value.
 */
int lcd_demo_run(const struct device *display_dev);
