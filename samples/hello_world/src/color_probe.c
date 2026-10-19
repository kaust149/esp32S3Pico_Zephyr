/*
 * Colour probe. See color_probe.h.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/drivers/display.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>

#include "color_probe.h"
#include "lcd.h" /* LCD_SWAP_RB */

#define PANEL_W   320
#define PANEL_H   320
#define CELL      40
#define NCOLORS   8
#define MAX_BYTES (PANEL_W * 8 * 3) /* largest strip written in one call */

struct rgb {
	uint8_t r, g, b;
};

/* Left to right in every row. */
static const struct rgb colors[NCOLORS] = {
	{0, 0, 0},     {255, 0, 0},   {0, 255, 0},   {0, 0, 255},
	{0, 255, 255}, {255, 0, 255}, {255, 255, 0}, {255, 255, 255},
};

static uint8_t buf[MAX_BYTES > CELL * CELL * 3 ? MAX_BYTES : CELL * CELL * 3];
static uint8_t big[PANEL_W * CELL * 3];
static enum display_pixel_format fmt;
static int bpp;

static void put(uint8_t *p, struct rgb c)
{
	if (LCD_SWAP_RB) {
		uint8_t t = c.r;

		c.r = c.b;
		c.b = t;
	}
	if (fmt == PIXEL_FORMAT_RGB_565) {
		uint16_t v = (uint16_t)(((c.r & 0xF8) << 8) | ((c.g & 0xFC) << 3) | (c.b >> 3));

		p[0] = (uint8_t)(v >> 8);
		p[1] = (uint8_t)(v & 0xFF);
	} else {
		p[0] = c.r;
		p[1] = c.g;
		p[2] = c.b;
	}
}

/* Logical (x, y) with (0, 0) top-left as the user sees it: flip like the LCD layer. */
static int write_rect(const struct device *dev, int x, int y, int w, int h, const uint8_t *data)
{
	const struct display_buffer_descriptor d = {
		.buf_size = (uint32_t)w * h * bpp,
		.width = w,
		.height = h,
		.pitch = w,
	};

	return display_write(dev, PANEL_W - x - w, PANEL_H - y - h, &d, data);
}

/* Uniform cell written in pieces of (pw x ph). */
static int cell(const struct device *dev, int x, int y, struct rgb c, int pw, int ph)
{
	for (int i = 0; i < pw * ph; i++) {
		put(&buf[i * bpp], c);
	}
	for (int yy = 0; yy < CELL; yy += ph) {
		for (int xx = 0; xx < CELL; xx += pw) {
			int r = write_rect(dev, x + xx, y + yy, pw, ph, buf);

			if (r < 0) {
				return r;
			}
		}
	}
	return 0;
}

int color_probe_run(const struct device *dev)
{
	struct display_capabilities caps;
	int r;

	display_get_capabilities(dev, &caps);
	fmt = caps.current_pixel_format;
	bpp = fmt == PIXEL_FORMAT_RGB_888 ? 3 : (fmt == PIXEL_FORMAT_RGB_565 ? 2 : 0);
	if (bpp == 0) {
		return -ENOTSUP;
	}
	printk("color probe: %s, rows A..G, cells K R G B C M Y W\n",
	       bpp == 3 ? "RGB888" : "RGB565");

	for (int c = 0; c < NCOLORS; c++) {
		const int x = c * CELL;

		/* A: one write per cell. */
		r = cell(dev, x, 0 * CELL, colors[c], CELL, CELL);
		/* B: 40-wide, 8 rows per write. */
		r = r < 0 ? r : cell(dev, x, 1 * CELL, colors[c], CELL, 8);
		/* C: 40-wide, 1 row per write. */
		r = r < 0 ? r : cell(dev, x, 2 * CELL, colors[c], CELL, 1);
		/* D: 8-wide, 40 rows per write. */
		r = r < 0 ? r : cell(dev, x, 3 * CELL, colors[c], 8, CELL);
		/* F: 1-pixel-wide columns. */
		r = r < 0 ? r : cell(dev, x, 5 * CELL, colors[c], 1, CELL);
		if (r < 0) {
			printk("color probe: write failed (%d)\n", r);
			return r;
		}
	}

	/* E: 320x8 strips holding all eight colours. */
	for (int row = 0; row < 8; row++) {
		for (int x = 0; x < PANEL_W; x++) {
			/* Panel x runs right to left, so the colour order is reversed here. */
			put(&buf[(row * PANEL_W + x) * bpp], colors[(PANEL_W - 1 - x) / CELL]);
		}
	}
	for (int yy = 0; yy < CELL; yy += 8) {
		r = write_rect(dev, 0, 4 * CELL + yy, PANEL_W, 8, buf);
		if (r < 0) {
			return r;
		}
	}

	/* G: one 320x40 write holding all eight colours. */
	for (int row = 0; row < CELL; row++) {
		for (int x = 0; x < PANEL_W; x++) {
			put(&big[(row * PANEL_W + x) * bpp], colors[(PANEL_W - 1 - x) / CELL]);
		}
	}
	r = write_rect(dev, 0, 6 * CELL, PANEL_W, CELL, big);
	printk("color probe done\n");
	return r < 0 ? r : 0;
}
