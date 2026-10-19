/*
 * PicoCalc key events to NumWorks key state.
 *
 * Turns the PicoCalc southbridge's key events (state + code) into the set of
 * NumWorks keys currently held down, as the 64-bit bitmap Epsilon's
 * Ion::Keyboard::State uses (bit = row * 6 + column). No hardware access here, so
 * it can be tested on a computer.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

/* Southbridge event states. */
#define KEYS_EVENT_PRESSED  1
#define KEYS_EVENT_HOLD     2
#define KEYS_EVENT_RELEASED 3

void keys_core_reset(void);

/* Feed one southbridge event. Unknown keys and the idle state (0) are ignored. */
void keys_core_event(uint8_t state, uint8_t code);

/* Keys currently held, as an Epsilon Ion keyboard state bitmap. */
uint64_t keys_core_state(void);

/* Write the names of the keys in `state` (for example "Shift+Sine") into buf. */
size_t keys_core_describe(uint64_t state, char *buf, size_t size);
