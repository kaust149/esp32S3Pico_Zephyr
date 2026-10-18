/*
 * PicoCalc on ESP32-S3-Pico (Zephyr): minimal bring-up application.
 *
 * Brings up the display and the PicoCalc southbridge keyboard and prints key
 * codes on the UART console. This is the base for the Epsilon (NumWorks) port.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/kernel.h>

#include "keyboard.h"
#include "southbridge.h"

#define POLL_INTERVAL_MS 10

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

	keyboard_init();

	while (1) {
		if (sb_available()) {
			keyboard_poll();
		}

		while (keyboard_key_available()) {
			unsigned char key = (unsigned char)keyboard_get_key();

			printk("key 0x%02x\n", key);
		}

		k_msleep(POLL_INTERVAL_MS);
	}

	return 0;
}
