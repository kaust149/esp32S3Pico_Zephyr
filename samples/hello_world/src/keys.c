/*
 * PicoCalc keys for Epsilon. See keys.h.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>

#include <zephyr/kernel.h>

#include "keys.h"
#include "southbridge.h"

/* Set to 1 to print every southbridge event (state, code) on the console. */
#define KEYS_DEBUG_EVENTS 1

/* Enough to drain a burst of key changes in one go without hogging the I2C bus. */
#define MAX_EVENTS_PER_POLL 16

void keys_init(void)
{
	sb_init();
	keys_core_reset();
}

int keys_poll(void)
{
	int n = 0;

	while (n < MAX_EVENTS_PER_POLL && sb_available()) {
		const uint16_t ev = sb_read_keyboard();
		const uint8_t state = (uint8_t)(ev >> 8);
		const uint8_t code = (uint8_t)(ev & 0xFF);

		if (state == 0) {
			break; /* FIFO empty (or an I2C error) */
		}
#if KEYS_DEBUG_EVENTS
		printk("sb event: state %u code 0x%02x\n", state, code);
#endif
		keys_core_event(state, code);
		n++;
	}
	return n;
}
