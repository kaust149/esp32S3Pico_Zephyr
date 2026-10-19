/*
 * PicoCalc key events to NumWorks key state. See keys_core.h.
 *
 * Layout (PicoCalc key -> NumWorks key):
 *   arrows -> arrows, Enter -> OK, Esc -> Back, Home/F1 -> Home,
 *   Backspace/Del -> Backspace, either Shift -> Shift, Alt -> Alpha, Power -> OnOff,
 *   0-9 . , + - * / ( ) ^ -> the same keys, '=' -> EXE,
 *   s c t -> Sine Cosine Tangent,  l g e -> Ln Log Exp,  p r q -> Pi Sqrt Square,
 *   i x v b -> Imaginary XNT Var Toolbox,  a n -> Ans EE.
 * Letters work in either case, because Shift changes the letter the southbridge reports.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "keys_core.h"
#include "numworks_keys.h"

/* PicoCalc key codes (see keyboard.h). */
#define PC_BACKSPACE 0x08
#define PC_ENTER     0x0A
#define PC_RETURN    0x0D
#define PC_F1        0x81
#define PC_POWER     0x91
#define PC_MOD_ALT   0xA1
#define PC_MOD_SHL   0xA2
#define PC_MOD_SHR   0xA3
#define PC_ESC       0xB1
#define PC_LEFT      0xB4
#define PC_UP        0xB5
#define PC_DOWN      0xB6
#define PC_RIGHT     0xB7
#define PC_HOME      0xD2
#define PC_DEL       0xD4

struct map {
	uint8_t code;
	uint8_t key; /* enum nw_key */
};

static const struct map key_map[] = {
	{PC_LEFT, NW_Left}, {PC_UP, NW_Up}, {PC_DOWN, NW_Down}, {PC_RIGHT, NW_Right},
	{PC_ENTER, NW_OK}, {PC_RETURN, NW_OK}, {PC_ESC, NW_Back},
	{PC_HOME, NW_Home}, {PC_F1, NW_Home},
	{PC_BACKSPACE, NW_Backspace}, {PC_DEL, NW_Backspace},
	{PC_MOD_ALT, NW_Alpha},
	{PC_POWER, NW_OnOff},
	{'0', NW_Zero}, {'1', NW_One}, {'2', NW_Two}, {'3', NW_Three}, {'4', NW_Four},
	{'5', NW_Five}, {'6', NW_Six}, {'7', NW_Seven}, {'8', NW_Eight}, {'9', NW_Nine},
	{'.', NW_Dot}, {',', NW_Comma}, {'+', NW_Plus}, {'-', NW_Minus},
	{'*', NW_Multiplication}, {'/', NW_Division},
	{'(', NW_LeftParenthesis}, {')', NW_RightParenthesis}, {'^', NW_Power},
	{'=', NW_EXE},
	{'s', NW_Sine}, {'c', NW_Cosine}, {'t', NW_Tangent},
	{'l', NW_Ln}, {'g', NW_Log}, {'e', NW_Exp},
	{'p', NW_Pi}, {'r', NW_Sqrt}, {'q', NW_Square},
	{'i', NW_Imaginary}, {'x', NW_XNT}, {'v', NW_Var}, {'b', NW_Toolbox},
	{'a', NW_Ans}, {'n', NW_EE},
};

/* With Shift held the southbridge reports the shifted symbol, but if Shift is let go
 * before the key, the release comes with the plain symbol. Each pair here is the same
 * physical key, so both codes must clear both keys, otherwise a key can stay stuck. */
static const struct {
	uint8_t a, b;
} same_physical_key[] = {
	{'(', '9'}, {')', '0'}, {'*', '8'}, {'+', '='}, {'^', '6'},
};

/* NumWorks' Shift is a tap, then the next key. The PicoCalc Shift is held while the next
 * key is typed, and symbols such as ^ or + already arrive as the shifted character. So
 * the held Shift is never passed on. It only becomes a short Shift tap when it was
 * pressed and released without any other key in between. */
#define SHIFT_TAP_POLLS 8
static _Atomic bool shift_down;
static _Atomic bool shift_used;
static _Atomic int shift_tap;

static void shift_event(uint8_t state)
{
	if (state == KEYS_EVENT_PRESSED) {
		atomic_store(&shift_used, false);
		atomic_store(&shift_down, true);
	} else if (state == KEYS_EVENT_RELEASED) {
		atomic_store(&shift_down, false);
		if (!atomic_load(&shift_used)) {
			atomic_store(&shift_tap, SHIFT_TAP_POLLS);
		}
	}
}

static _Atomic uint32_t state_lo; /* keys 0..31 */
static _Atomic uint32_t state_hi; /* keys 32..63 */

static int lookup(uint8_t code)
{
	if (code >= 'A' && code <= 'Z') {
		code = (uint8_t)(code + ('a' - 'A'));
	}
	for (size_t i = 0; i < sizeof(key_map) / sizeof(key_map[0]); i++) {
		if (key_map[i].code == code) {
			return key_map[i].key;
		}
	}
	return -1;
}

static uint8_t sibling(uint8_t code)
{
	for (size_t i = 0; i < sizeof(same_physical_key) / sizeof(same_physical_key[0]); i++) {
		if (same_physical_key[i].a == code) {
			return same_physical_key[i].b;
		}
		if (same_physical_key[i].b == code) {
			return same_physical_key[i].a;
		}
	}
	return 0;
}

static void set_key(int key, bool down)
{
	_Atomic uint32_t *word = key < 32 ? &state_lo : &state_hi;
	const uint32_t bit = (uint32_t)1 << (key & 31);

	if (down) {
		atomic_fetch_or(word, bit);
	} else {
		atomic_fetch_and(word, ~bit);
	}
}

void keys_core_reset(void)
{
	atomic_store(&state_lo, 0);
	atomic_store(&state_hi, 0);
	atomic_store(&shift_down, false);
	atomic_store(&shift_used, false);
	atomic_store(&shift_tap, 0);
}

void keys_core_event(uint8_t state, uint8_t code)
{
	int key;

	if (code == PC_MOD_SHL || code == PC_MOD_SHR) {
		shift_event(state);
		return;
	}
	key = lookup(code);
	if (state == KEYS_EVENT_PRESSED && atomic_load(&shift_down)) {
		atomic_store(&shift_used, true);
	}

	if (state == KEYS_EVENT_PRESSED || state == KEYS_EVENT_HOLD) {
		if (key >= 0) {
			set_key(key, true);
		}
	} else if (state == KEYS_EVENT_RELEASED) {
		if (key >= 0) {
			set_key(key, false);
		}
		uint8_t other = sibling(code);

		if (other != 0 && lookup(other) >= 0) {
			set_key(lookup(other), false);
		}
	}
}

uint64_t keys_core_state(void)
{
	/* Each word is read on its own: the keys are independent bits, so a snapshot
	 * taken across an update is still a valid combination of held keys. */
	const uint64_t hi = atomic_load(&state_hi);
	const uint64_t lo = atomic_load(&state_lo);

	uint64_t all = (hi << 32) | lo;
	int tap = atomic_load(&shift_tap);

	if (tap > 0) {
		atomic_store(&shift_tap, tap - 1);
		all |= (uint64_t)1 << NW_Shift;
	}
	return all;
}

size_t keys_core_describe(uint64_t state, char *buf, size_t size)
{
	size_t n = 0;

	if (size == 0) {
		return 0;
	}
	buf[0] = '\0';
	for (int k = 0; k < NW_KEY_SLOTS; k++) {
		if (((state >> k) & 1) && nw_key_names[k] != NULL) {
			int w = snprintf(buf + n, size - n, "%s%s", n ? "+" : "", nw_key_names[k]);

			if (w < 0 || (size_t)w >= size - n) {
				break;
			}
			n += (size_t)w;
		}
	}
	return n;
}
