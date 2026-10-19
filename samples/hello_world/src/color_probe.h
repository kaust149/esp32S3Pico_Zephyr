/*
 * Colour probe: draws the same eight colours with different write sizes so that a
 * colour problem can be traced to the way pixels are sent. Uses display_write()
 * directly, without the LCD layer.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <zephyr/device.h>

/*
 * Draws seven rows, 40 pixels tall each, top to bottom. Every row shows eight
 * 40-pixel-wide cells, left to right:
 *   black, red, green, blue, cyan, magenta, yellow, white
 * Rows differ only in how the pixels are sent:
 *   A: one write per cell (40x40)      E: 320x8 strips, all colours in one write
 *   B: 40-wide, 8 rows per write       F: 1-pixel-wide columns
 *   C: 40-wide, 1 row per write        G: one 320x40 write, all colours
 *   D: 8-wide, 40 rows per write
 */
int color_probe_run(const struct device *display_dev);
