/*
 * Key layer check: a background thread polls the keys and prints the NumWorks keys
 * held down whenever they change. This is also how Epsilon's keyboard backend will
 * be driven.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>

#include <zephyr/kernel.h>

#include "keys.h"

#define KEYS_POLL_MS 10

/* With Epsilon the keys are read by its Ion backend instead. */
#ifndef CONFIG_EPSILON

static void keys_demo_thread(void *a, void *b, void *c)
{
	uint64_t last = 0;
	char names[96];

	(void)a;
	(void)b;
	(void)c;

	keys_init();
	printk("keys: ready, press keys\n");

	while (1) {
		keys_poll();

		const uint64_t now = keys_state();

		if (now != last) {
			keys_core_describe(now, names, sizeof(names));
			printk("keys held: %08x%08x  %s\n", (unsigned int)(now >> 32),
			       (unsigned int)now, names[0] ? names : "(none)");
			last = now;
		}
		k_msleep(KEYS_POLL_MS);
	}
}

K_THREAD_DEFINE(keys_demo_tid, 4096, keys_demo_thread, NULL, NULL, NULL, 7, 0, 0);

#endif /* !CONFIG_EPSILON */
