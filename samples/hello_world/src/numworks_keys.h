/*
 * NumWorks Epsilon key numbers, generated from Epsilon's
 * shared/ion/include/ion/keyboard/epsilon/ion/keys.inc. Bit N of the Ion keyboard
 * state is key N = row * NW_KEY_COLUMNS + column. Included by keys_core.c only.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#define NW_KEY_ROWS    9
#define NW_KEY_COLUMNS 6
#define NW_KEY_SLOTS   (NW_KEY_ROWS * NW_KEY_COLUMNS)

enum nw_key {
	NW_Left = 0, /* row 0, column 0 */
	NW_Up = 1, /* row 0, column 1 */
	NW_Down = 2, /* row 0, column 2 */
	NW_Right = 3, /* row 0, column 3 */
	NW_OK = 4, /* row 0, column 4 */
	NW_Back = 5, /* row 0, column 5 */
	NW_Home = 6, /* row 1, column 0 */
	NW_OnOff = 8, /* row 1, column 2 */
	NW_Shift = 12, /* row 2, column 0 */
	NW_Alpha = 13, /* row 2, column 1 */
	NW_XNT = 14, /* row 2, column 2 */
	NW_Var = 15, /* row 2, column 3 */
	NW_Toolbox = 16, /* row 2, column 4 */
	NW_Backspace = 17, /* row 2, column 5 */
	NW_Exp = 18, /* row 3, column 0 */
	NW_Ln = 19, /* row 3, column 1 */
	NW_Log = 20, /* row 3, column 2 */
	NW_Imaginary = 21, /* row 3, column 3 */
	NW_Comma = 22, /* row 3, column 4 */
	NW_Power = 23, /* row 3, column 5 */
	NW_Sine = 24, /* row 4, column 0 */
	NW_Cosine = 25, /* row 4, column 1 */
	NW_Tangent = 26, /* row 4, column 2 */
	NW_Pi = 27, /* row 4, column 3 */
	NW_Sqrt = 28, /* row 4, column 4 */
	NW_Square = 29, /* row 4, column 5 */
	NW_Seven = 30, /* row 5, column 0 */
	NW_Eight = 31, /* row 5, column 1 */
	NW_Nine = 32, /* row 5, column 2 */
	NW_LeftParenthesis = 33, /* row 5, column 3 */
	NW_RightParenthesis = 34, /* row 5, column 4 */
	NW_Four = 36, /* row 6, column 0 */
	NW_Five = 37, /* row 6, column 1 */
	NW_Six = 38, /* row 6, column 2 */
	NW_Multiplication = 39, /* row 6, column 3 */
	NW_Division = 40, /* row 6, column 4 */
	NW_One = 42, /* row 7, column 0 */
	NW_Two = 43, /* row 7, column 1 */
	NW_Three = 44, /* row 7, column 2 */
	NW_Plus = 45, /* row 7, column 3 */
	NW_Minus = 46, /* row 7, column 4 */
	NW_Zero = 48, /* row 8, column 0 */
	NW_Dot = 49, /* row 8, column 1 */
	NW_EE = 50, /* row 8, column 2 */
	NW_Ans = 51, /* row 8, column 3 */
	NW_EXE = 52, /* row 8, column 4 */
};

static const char *const nw_key_names[NW_KEY_SLOTS] = {
	[NW_Left] = "Left",
	[NW_Up] = "Up",
	[NW_Down] = "Down",
	[NW_Right] = "Right",
	[NW_OK] = "OK",
	[NW_Back] = "Back",
	[NW_Home] = "Home",
	[NW_OnOff] = "OnOff",
	[NW_Shift] = "Shift",
	[NW_Alpha] = "Alpha",
	[NW_XNT] = "XNT",
	[NW_Var] = "Var",
	[NW_Toolbox] = "Toolbox",
	[NW_Backspace] = "Backspace",
	[NW_Exp] = "Exp",
	[NW_Ln] = "Ln",
	[NW_Log] = "Log",
	[NW_Imaginary] = "Imaginary",
	[NW_Comma] = "Comma",
	[NW_Power] = "Power",
	[NW_Sine] = "Sine",
	[NW_Cosine] = "Cosine",
	[NW_Tangent] = "Tangent",
	[NW_Pi] = "Pi",
	[NW_Sqrt] = "Sqrt",
	[NW_Square] = "Square",
	[NW_Seven] = "Seven",
	[NW_Eight] = "Eight",
	[NW_Nine] = "Nine",
	[NW_LeftParenthesis] = "LeftParenthesis",
	[NW_RightParenthesis] = "RightParenthesis",
	[NW_Four] = "Four",
	[NW_Five] = "Five",
	[NW_Six] = "Six",
	[NW_Multiplication] = "Multiplication",
	[NW_Division] = "Division",
	[NW_One] = "One",
	[NW_Two] = "Two",
	[NW_Three] = "Three",
	[NW_Plus] = "Plus",
	[NW_Minus] = "Minus",
	[NW_Zero] = "Zero",
	[NW_Dot] = "Dot",
	[NW_EE] = "EE",
	[NW_Ans] = "Ans",
	[NW_EXE] = "EXE",
};
