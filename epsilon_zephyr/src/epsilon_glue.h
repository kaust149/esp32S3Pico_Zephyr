/*
 * Small C interface between Epsilon's Ion backend (C++) and the Zephyr side.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint64_t epsilon_glue_millis(void);
void epsilon_glue_msleep(uint32_t ms);
uint32_t epsilon_glue_random(void);
void epsilon_glue_putchar(char c);

/* Poll the keys and return the held-key bitmap (Epsilon Ion keyboard state). */
uint64_t epsilon_glue_scan_keys(void);

/* Screen, provided by the application's lcd layer (lcd.h). */
void lcd_push_rect(int x, int y, int w, int h, const uint16_t *pixels);
void lcd_push_uniform(int x, int y, int w, int h, uint16_t color);
void lcd_pull_rect(int x, int y, int w, int h, uint16_t *pixels);

#ifdef __cplusplus
}
#endif
