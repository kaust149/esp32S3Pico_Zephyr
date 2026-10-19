/*
 * Display bring-up test for the PicoCalc, drawing directly with display_write().
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <zephyr/device.h>

/*
 * Draws, in order:
 *   1. full-screen solid red, green and blue (about 0.7 s each), printing how
 *      long each full-screen push took on the console;
 *   2. eight horizontal colour bands, top to bottom: white, yellow, cyan,
 *      green, magenta, red, blue, black. A black square marks the top-left
 *      corner and a white square marks the bottom-right corner.
 *
 * Supports the RGB888 and RGB565 pixel formats. Returns 0 on success or a
 * negative errno value.
 */
int display_test_run(const struct device *display_dev);
