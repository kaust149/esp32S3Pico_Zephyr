/*
 * On-board check of the LCD layer (lcd.h). See lcd_demo.h.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <zephyr/kernel.h>

#include "lcd.h"
#include "lcd_demo.h"

#ifdef CONFIG_ESP_SPIRAM
#define IN_PSRAM __attribute__((section(".ext_ram.bss.lcd_demo")))
#else
#define IN_PSRAM
#endif

#define RAMP_ROWS 60

static uint16_t image[LCD_WIDTH * LCD_HEIGHT] IN_PSRAM;
static uint16_t readback[LCD_WIDTH * LCD_HEIGHT] IN_PSRAM;

static uint16_t rgb565(unsigned int r, unsigned int g, unsigned int b)
{
	return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

static void build_image(void)
{
	for (int y = 0; y < LCD_HEIGHT; y++) {
		for (int x = 0; x < LCD_WIDTH; x++) {
			const unsigned int v = (unsigned int)(x * 255 / (LCD_WIDTH - 1));
			uint16_t c;

			switch (y / RAMP_ROWS) {
			case 0:
				c = rgb565(v, 0, 0);
				break;
			case 1:
				c = rgb565(0, v, 0);
				break;
			case 2:
				c = rgb565(0, 0, v);
				break;
			default:
				c = rgb565(v, v, v);
				break;
			}
			image[y * LCD_WIDTH + x] = c;
		}
	}
}

static unsigned int ms_since(int64_t t0)
{
	return (unsigned int)(k_uptime_get() - t0);
}

int lcd_demo_run(const struct device *display_dev)
{
	static uint16_t block[100 * 60];
	int ret = lcd_init(display_dev);

	if (ret < 0) {
		printk("lcd demo: lcd_init failed (%d)\n", ret);
		return ret;
	}

	build_image();

	/* 1. Whole screen from an RGB565 buffer, the way Epsilon pushes a full frame. */
	int64_t t0 = k_uptime_get();

	lcd_push_rect(0, 0, LCD_WIDTH, LCD_HEIGHT, image);
	printk("lcd demo: full 320x240 push_rect took %u ms\n", ms_since(t0));

	/* 2. Frame and corner markers as uniform fills. */
	t0 = k_uptime_get();
	lcd_push_uniform(0, 0, LCD_WIDTH, 1, 0xFFFF);
	lcd_push_uniform(0, LCD_HEIGHT - 1, LCD_WIDTH, 1, 0xFFFF);
	lcd_push_uniform(0, 0, 1, LCD_HEIGHT, 0xFFFF);
	lcd_push_uniform(LCD_WIDTH - 1, 0, 1, LCD_HEIGHT, 0xFFFF);
	lcd_push_uniform(1, 1, 16, 16, rgb565(255, 0, 0));
	lcd_push_uniform(LCD_WIDTH - 17, 1, 16, 16, rgb565(0, 255, 0));
	lcd_push_uniform(1, LCD_HEIGHT - 17, 16, 16, rgb565(0, 0, 255));
	lcd_push_uniform(LCD_WIDTH - 17, LCD_HEIGHT - 17, 16, 16, 0xFFFF);
	printk("lcd demo: frame and markers (8 uniform fills) took %u ms\n", ms_since(t0));

	/* 3. Partial updates in the middle: a checker block and a uniform block. */
	for (int y = 0; y < 60; y++) {
		for (int x = 0; x < 100; x++) {
			block[y * 100 + x] = (((x / 10) + (y / 10)) & 1) ? 0xFFFF : 0x0000;
		}
	}
	t0 = k_uptime_get();
	lcd_push_rect(110, 90, 100, 60, block);
	printk("lcd demo: 100x60 push_rect took %u ms\n", ms_since(t0));

	t0 = k_uptime_get();
	lcd_push_uniform(135, 105, 50, 30, rgb565(255, 160, 0));
	printk("lcd demo: 50x30 push_uniform took %u ms\n", ms_since(t0));

	/* 4. Read the whole screen back and check what was drawn. */
	int bad = 0;

	lcd_pull_rect(0, 0, LCD_WIDTH, LCD_HEIGHT, readback);
	for (int y = 0; y < LCD_HEIGHT; y++) {
		for (int x = 0; x < LCD_WIDTH; x++) {
			uint16_t want = image[y * LCD_WIDTH + x];

			if (x == 0 || y == 0 || x == LCD_WIDTH - 1 || y == LCD_HEIGHT - 1) {
				want = 0xFFFF;
			} else if (x >= 1 && x < 17 && y >= 1 && y < 17) {
				want = rgb565(255, 0, 0);
			} else if (x >= LCD_WIDTH - 17 && x < LCD_WIDTH - 1 && y >= 1 && y < 17) {
				want = rgb565(0, 255, 0);
			} else if (x >= 1 && x < 17 && y >= LCD_HEIGHT - 17 && y < LCD_HEIGHT - 1) {
				want = rgb565(0, 0, 255);
			} else if (x >= LCD_WIDTH - 17 && x < LCD_WIDTH - 1 &&
				   y >= LCD_HEIGHT - 17 && y < LCD_HEIGHT - 1) {
				want = 0xFFFF;
			} else if (x >= 135 && x < 185 && y >= 105 && y < 135) {
				want = rgb565(255, 160, 0);
			} else if (x >= 110 && x < 210 && y >= 90 && y < 150) {
				want = block[(y - 90) * 100 + (x - 110)];
			}
			if (readback[y * LCD_WIDTH + x] != want) {
				bad++;
			}
		}
	}
	printk("lcd demo: read-back check %s (%d mismatching pixels)\n",
	       bad == 0 ? "OK" : "FAILED", bad);

	return bad == 0 ? 0 : -EIO;
}
