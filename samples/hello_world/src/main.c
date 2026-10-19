/*
 * PicoCalc on ESP32-S3-Pico (Zephyr): minimal bring-up application.
 *
 * Brings up the display and runs the LCD and colour checks. Keys are handled by
 * keys_demo.c. This is the base for the Epsilon (NumWorks) port.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/kernel.h>

#include "color_probe.h"
#include "display_test.h"
#include "keys.h"
#include "lcd.h"
#include "lcd_demo.h"

#ifdef CONFIG_EPSILON
extern void epsilon_start(void);
#endif

int main(void)
{
	const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

	if (!device_is_ready(display_dev)) {
		printk("Display not ready, aborting\n");
		return 0;
	}

	k_sleep(K_MSEC(100));
	display_blanking_off(display_dev);
	printk("PicoCalc ready on %s\n", CONFIG_BOARD_TARGET);

#ifdef CONFIG_EPSILON
	/* Run NumWorks Epsilon. It never returns. */
	if (lcd_init(display_dev) < 0) {
		printk("LCD init failed\n");
		return 0;
	}
	keys_init();
	printk("Starting Epsilon\n");
	epsilon_start();
	return 0;
#endif

	/* Display bring-up test; remove once the display path is verified. */
	if (display_test_run(display_dev) < 0) {
		printk("Display test failed\n");
	}

	/* LCD layer check (Epsilon-style RGB565 drawing); runs after the raw test. */
	k_sleep(K_MSEC(1500));
	if (lcd_demo_run(display_dev) < 0) {
		printk("LCD demo failed\n");
	}

	/* Colour probe: shown a few seconds after the LCD demo. */
	k_sleep(K_MSEC(8000));
	if (color_probe_run(display_dev) < 0) {
		printk("Colour probe failed\n");
	}

	/* Keys are handled by the thread in keys_demo.c, started at boot. */
	while (1) {
		k_sleep(K_FOREVER);
	}

	return 0;
}
