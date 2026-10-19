/*
 * PicoCalc keys for Epsilon: polls the southbridge and keeps the held-key bitmap.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>

#include "keys_core.h"

/* Bring up the southbridge and clear the held-key state. */
void keys_init(void);

/* Read pending southbridge key events and update the state. Returns how many were read. */
int keys_poll(void);

/* Keys held right now, as an Epsilon Ion keyboard state bitmap. */
static inline uint64_t keys_state(void)
{
	return keys_core_state();
}
