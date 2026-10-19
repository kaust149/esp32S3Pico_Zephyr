/*
 * PicoCalc LCD layer for the Epsilon (NumWorks) port. See lcd.h.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <zephyr/drivers/display.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>

#include "lcd.h"

/* Physical panel: 320x320 shown part of the ILI9488's 320x480 memory. */
#define PANEL_W 320
#define PANEL_H 320

/* The panel is mounted rotated 180 degrees against the controller's scan, and the
 * driver's own rotation shifts the picture, so the flip is done here. */
#define FLIP_X 0
#define FLIP_Y 0

/* Epsilon's 240 rows are centred on the 320-row panel. */
#define Y_OFFSET ((PANEL_H - LCD_HEIGHT) / 2)

/* Widest wire format: RGB888, 3 bytes per pixel. The chunk buffer is sized for it. */
#define MAX_BPP 3

/* Pixels converted and sent per display_write(). Internal RAM, so SPI DMA can use it. */
#define CHUNK_PIXELS 8192

#ifdef CONFIG_ESP_SPIRAM
#define IN_PSRAM __attribute__((section(".ext_ram.bss.lcd_shadow")))
#else
#define IN_PSRAM
#endif

static const struct device *dev;
static uint16_t shadow[LCD_WIDTH * LCD_HEIGHT] IN_PSRAM;
static uint8_t chunk[CHUNK_PIXELS * MAX_BPP];

/* Wire format the panel was configured with (devicetree pixel-format). */
static enum display_pixel_format wire_format;
static int bpp;

/* Encode one RGB565 pixel for the panel (red and blue swapped, see LCD_SWAP_RB).
 * RGB888: the panel only uses the top 6 bits of each byte, and the low bits are
 * filled so white stays white. RGB565: Zephyr's big-endian byte order. */
static inline void encode_pixel(uint16_t v, uint8_t *out)
{
	if (LCD_SWAP_RB) {
		/* Swap the 5-bit red and blue fields of the RGB565 value. */
		v = (uint16_t)(((v & 0x1F) << 11) | (v & 0x07E0) | (v >> 11));
	}
	if (wire_format == PIXEL_FORMAT_RGB_565) {
		out[0] = (uint8_t)(v >> 8);
		out[1] = (uint8_t)(v & 0xFF);
	} else {
		uint8_t r = (v >> 8) & 0xF8;
		uint8_t g = (v >> 3) & 0xFC;
		uint8_t b = (v << 3) & 0xF8;

		out[0] = r | (r >> 5);
		out[1] = g | (g >> 6);
		out[2] = b | (b >> 5);
	}
}

/* Intersect a logical rectangle with the screen. Returns false when nothing is left. */
static bool clip(int *x, int *y, int *w, int *h, int *skip_x, int *skip_y)
{
	int x0 = *x, y0 = *y, x1 = *x + *w, y1 = *y + *h;

	if (*w <= 0 || *h <= 0) {
		return false;
	}
	if (x0 < 0) {
		x0 = 0;
	}
	if (y0 < 0) {
		y0 = 0;
	}
	if (x1 > LCD_WIDTH) {
		x1 = LCD_WIDTH;
	}
	if (y1 > LCD_HEIGHT) {
		y1 = LCD_HEIGHT;
	}
	if (x0 >= x1 || y0 >= y1) {
		return false;
	}
	*skip_x = x0 - *x;
	*skip_y = y0 - *y;
	*x = x0;
	*y = y0;
	*w = x1 - x0;
	*h = y1 - y0;
	return true;
}

/* Physical top-left and size of a logical rectangle on the panel. */
static void to_panel(int x, int y, int w, int h, int *px, int *py)
{
	const int ly = y + Y_OFFSET;

	*px = FLIP_X ? PANEL_W - x - w : x;
	*py = FLIP_Y ? PANEL_H - ly - h : ly;
}

static int panel_write(int px, int py, int w, int rows)
{
	const struct display_buffer_descriptor desc = {
		.buf_size = (uint32_t)w * rows * bpp,
		.width = w,
		.height = rows,
		.pitch = w,
	};

	return display_write(dev, px, py, &desc, chunk);
}

/* Send a logical rectangle of the shadow copy to the panel. */
static void flush_shadow(int x, int y, int w, int h)
{
	int px, py;

	to_panel(x, y, w, h, &px, &py);

	const int rows_per_chunk = MAX(1, CHUNK_PIXELS / w);

	for (int done = 0; done < h; done += rows_per_chunk) {
		const int rows = MIN(rows_per_chunk, h - done);

		for (int r = 0; r < rows; r++) {
			/* Panel rows run top to bottom; with FLIP_Y that is logical bottom to top. */
			const int ly = FLIP_Y ? y + h - 1 - (done + r) : y + done + r;
			const uint16_t *src = &shadow[ly * LCD_WIDTH + x];
			uint8_t *dst = &chunk[r * w * bpp];

			if (FLIP_X) {
				for (int i = 0; i < w; i++) {
					encode_pixel(src[w - 1 - i], &dst[i * bpp]);
				}
			} else {
				for (int i = 0; i < w; i++) {
					encode_pixel(src[i], &dst[i * bpp]);
				}
			}
		}

		panel_write(px, py + done, w, rows);
	}
}

/* Fill a physical panel rectangle with one RGB565 colour. */
static void panel_fill(int px, int py, int w, int h, uint16_t color)
{
	const int rows_per_chunk = MAX(1, CHUNK_PIXELS / w);
	const int fill_rows = MIN(rows_per_chunk, h);
	uint8_t pixel[MAX_BPP];

	encode_pixel(color, pixel);
	for (int i = 0; i < w * fill_rows; i++) {
		memcpy(&chunk[i * bpp], pixel, bpp);
	}
	for (int done = 0; done < h; done += rows_per_chunk) {
		panel_write(px, py + done, w, MIN(rows_per_chunk, h - done));
	}
}

int lcd_init(const struct device *display)
{
	struct display_capabilities caps;

	if (display == NULL || !device_is_ready(display)) {
		return -ENODEV;
	}
	display_get_capabilities(display, &caps);
	if (caps.current_pixel_format == PIXEL_FORMAT_RGB_888) {
		bpp = 3;
	} else if (caps.current_pixel_format == PIXEL_FORMAT_RGB_565) {
		bpp = 2;
	} else {
		return -ENOTSUP;
	}
	wire_format = caps.current_pixel_format;
	printk("lcd: wire format %s, %d bytes/pixel\n",
	       bpp == 3 ? "RGB888" : "RGB565", bpp);

	dev = display;
	memset(shadow, 0, sizeof(shadow));
	panel_fill(0, 0, PANEL_W, PANEL_H, 0x0000);
	return 0;
}

void lcd_push_rect(int x, int y, int w, int h, const uint16_t *pixels)
{
	const int src_w = w;
	int skip_x, skip_y;

	if (dev == NULL || !clip(&x, &y, &w, &h, &skip_x, &skip_y)) {
		return;
	}
	for (int r = 0; r < h; r++) {
		memcpy(&shadow[(y + r) * LCD_WIDTH + x],
		       &pixels[(skip_y + r) * src_w + skip_x], (size_t)w * sizeof(uint16_t));
	}
	flush_shadow(x, y, w, h);
}

void lcd_push_uniform(int x, int y, int w, int h, uint16_t color)
{
	int skip_x, skip_y, px, py;

	if (dev == NULL || !clip(&x, &y, &w, &h, &skip_x, &skip_y)) {
		return;
	}
	for (int r = 0; r < h; r++) {
		uint16_t *row = &shadow[(y + r) * LCD_WIDTH + x];

		for (int i = 0; i < w; i++) {
			row[i] = color;
		}
	}
	to_panel(x, y, w, h, &px, &py);
	panel_fill(px, py, w, h, color);
}

void lcd_pull_rect(int x, int y, int w, int h, uint16_t *pixels)
{
	const int out_w = w;
	int skip_x, skip_y;

	if (w > 0 && h > 0) {
		memset(pixels, 0, (size_t)w * h * sizeof(uint16_t));
	}
	if (dev == NULL || !clip(&x, &y, &w, &h, &skip_x, &skip_y)) {
		return;
	}
	for (int r = 0; r < h; r++) {
		memcpy(&pixels[(skip_y + r) * out_w + skip_x],
		       &shadow[(y + r) * LCD_WIDTH + x], (size_t)w * sizeof(uint16_t));
	}
}
