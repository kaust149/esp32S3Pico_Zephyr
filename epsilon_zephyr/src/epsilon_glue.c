/*
 * Zephyr side of the Epsilon Ion backend. See epsilon_glue.h.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "epsilon_glue.h"
#include "keys.h"

uint64_t epsilon_glue_millis(void)
{
	return (uint64_t)k_uptime_get();
}

void epsilon_glue_msleep(uint32_t ms)
{
	k_msleep(ms);
}

/* Small xorshift generator: Epsilon only needs it for its random() function, and this avoids
 * depending on an entropy driver. Seeded from the cycle counter on first use. */
uint32_t epsilon_glue_random(void)
{
	static uint32_t x;

	if (x == 0) {
		x = k_cycle_get_32() ^ (uint32_t)k_uptime_get() ^ 0x9E3779B9u;
		if (x == 0) {
			x = 1;
		}
	}
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return x;
}

void epsilon_glue_putchar(char c)
{
	printk("%c", c);
}

uint64_t epsilon_glue_scan_keys(void)
{
	keys_poll();
	return keys_state();
}
