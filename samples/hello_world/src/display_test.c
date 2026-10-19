/*
 * Display bring-up test for the PicoCalc, drawing directly with display_write().
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/drivers/display.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>

#include "display_test.h"

#define MAX_WIDTH    480 /* widest rectangle fill_rect() can write */
#define STRIP_LINES  8   /* rows sent per display_write() call */
#define MAX_BPP      3   /* RGB888 */
#define MARKER_SIZE  24
#define HOLD_MS      700

/*
 * The visible panel is 320x320, whatever the driver reports (the ILI9488 driver
 * reports its full 320x480 memory, of which only the first 320 rows are shown).
 *
 * The panel is mounted rotated 180 degrees relative to the controller's scan
 * with rotation = <0>. The driver's own rotation = <180> shifts the picture by
 * 160 rows on this panel, so the flip is done here instead: a logical rectangle
 * (x, y, w, h) is written at its mirrored position. If the picture is mirrored
 * left-right or upside down, change these two flags.
 */
#define PANEL_W  320
#define PANEL_H  320
#define FLIP_X   1
#define FLIP_Y   1

struct rgb {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

static const struct rgb k_black = {0, 0, 0};
static const struct rgb k_white = {255, 255, 255};

/* Colour bars, top to bottom. */
static const struct rgb k_bands[] = {
	{255, 255, 255}, /* white */
	{255, 255, 0},   /* yellow */
	{0, 255, 255},   /* cyan */
	{0, 255, 0},     /* green */
	{255, 0, 255},   /* magenta */
	{255, 0, 0},     /* red */
	{0, 0, 255},     /* blue */
	{0, 0, 0},       /* black */
};

static uint8_t strip[MAX_WIDTH * STRIP_LINES * MAX_BPP];

static int bytes_per_pixel(enum display_pixel_format fmt)
{
	switch (fmt) {
	case PIXEL_FORMAT_RGB_888:
		return 3;
	case PIXEL_FORMAT_RGB_565:
		return 2;
	default:
		return 0;
	}
}

static void put_pixel(uint8_t *p, enum display_pixel_format fmt, struct rgb c)
{
	if (fmt == PIXEL_FORMAT_RGB_888) {
		p[0] = c.r;
		p[1] = c.g;
		p[2] = c.b;
	} else {
		/* RGB565, big-endian byte order as documented by Zephyr. */
		uint16_t v = (uint16_t)(((c.r & 0xF8) << 8) | ((c.g & 0xFC) << 3) | (c.b >> 3));

		p[0] = (uint8_t)(v >> 8);
		p[1] = (uint8_t)(v & 0xFF);
	}
}

/* Fill a rectangle with one colour, STRIP_LINES rows per display_write(). */
static int fill_rect(const struct device *dev, enum display_pixel_format fmt,
		     uint16_t x, uint16_t y, uint16_t w, uint16_t h, struct rgb c)
{
	const int bpp = bytes_per_pixel(fmt);

	if (w == 0 || h == 0) {
		return 0;
	}
	if (bpp == 0 || w > MAX_WIDTH) {
		return -ENOTSUP;
	}

	for (uint32_t i = 0; i < (uint32_t)w * STRIP_LINES; i++) {
		put_pixel(&strip[i * bpp], fmt, c);
	}

	if (FLIP_X) {
		x = PANEL_W - x - w;
	}
	if (FLIP_Y) {
		y = PANEL_H - y - h;
	}

	for (uint16_t row = 0; row < h; row += STRIP_LINES) {
		const uint16_t lines = MIN(STRIP_LINES, h - row);
		const struct display_buffer_descriptor desc = {
			.buf_size = (uint32_t)w * lines * bpp,
			.width = w,
			.height = lines,
			.pitch = w,
		};
		int ret = display_write(dev, x, y + row, &desc, strip);

		if (ret < 0) {
			return ret;
		}
	}

	return 0;
}

int display_test_run(const struct device *display_dev)
{
	struct display_capabilities caps;
	int ret;

	display_get_capabilities(display_dev, &caps);

	const enum display_pixel_format fmt = caps.current_pixel_format;
	const uint16_t w = PANEL_W;
	const uint16_t h = PANEL_H;

	printk("display: driver reports %ux%u, using %ux%u, pixel format 0x%x, %d bytes/pixel\n",
	       (unsigned int)caps.x_resolution, (unsigned int)caps.y_resolution,
	       (unsigned int)w, (unsigned int)h, (unsigned int)fmt, bytes_per_pixel(fmt));

	if (bytes_per_pixel(fmt) == 0) {
		printk("display test: unsupported pixel format\n");
		return -ENOTSUP;
	}

	/* 1. Solid fills, timing each full-screen push. */
	static const struct rgb solid[] = {{255, 0, 0}, {0, 255, 0}, {0, 0, 255}};
	static const char *const names[] = {"red", "green", "blue"};

	for (size_t i = 0; i < ARRAY_SIZE(solid); i++) {
		const int64_t t0 = k_uptime_get();

		ret = fill_rect(display_dev, fmt, 0, 0, w, h, solid[i]);
		if (ret < 0) {
			printk("display test: %s fill failed (%d)\n", names[i], ret);
			return ret;
		}

		printk("%s fill: full-screen push took %u ms\n", names[i],
		       (unsigned int)(k_uptime_get() - t0));
		k_msleep(HOLD_MS);
	}

	/* 2. Colour bands plus corner markers. */
	const uint16_t band_h = h / ARRAY_SIZE(k_bands);

	for (size_t i = 0; i < ARRAY_SIZE(k_bands); i++) {
		const uint16_t y = (uint16_t)(i * band_h);
		const uint16_t bh = (i == ARRAY_SIZE(k_bands) - 1) ? (uint16_t)(h - y) : band_h;

		ret = fill_rect(display_dev, fmt, 0, y, w, bh, k_bands[i]);
		if (ret < 0) {
			printk("display test: band %u failed (%d)\n", (unsigned int)i, ret);
			return ret;
		}
	}

	ret = fill_rect(display_dev, fmt, 0, 0, MARKER_SIZE, MARKER_SIZE, k_black);
	if (ret < 0) {
		return ret;
	}
	ret = fill_rect(display_dev, fmt, w - MARKER_SIZE, h - MARKER_SIZE,
			MARKER_SIZE, MARKER_SIZE, k_white);
	if (ret < 0) {
		return ret;
	}

	/*
	 * 3. Ruler to measure any horizontal offset or dead strip: 8-pixel blocks
	 * across the full width, in two rows. Block 0 (left edge) is red and the
	 * last block (right edge) is green; the rest alternate white and black.
	 * The second row is offset by one block so the pattern is easy to count.
	 */
	const uint16_t nblocks = w / 8;

	for (uint16_t row = 0; row < 2; row++) {
		for (uint16_t i = 0; i < nblocks; i++) {
			struct rgb c = ((i + row) & 1) ? k_white : k_black;

			if (i == 0) {
				c = (struct rgb){255, 0, 0};
			} else if (i == nblocks - 1) {
				c = (struct rgb){0, 255, 0};
			}
			ret = fill_rect(display_dev, fmt, i * 8, 48 + row * 16, 8, 16, c);
			if (ret < 0) {
				return ret;
			}
		}
	}
	printk("ruler: %u blocks of 8 px, red = left edge, green = right edge\n", nblocks);

	printk("display test done\n");
	return 0;
}
