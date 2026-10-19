/* Epsilon expects the 32-bit limits to have the type of int32_t / uint32_t. This C library
 * writes them with an L suffix, which makes overloads on IntegerHandler ambiguous. */
#pragma once
#include <stdint.h>
#undef INT32_MAX
#undef INT32_MIN
#undef UINT32_MAX
#define INT32_MAX 2147483647
#define INT32_MIN (-2147483647 - 1)
#define UINT32_MAX 4294967295U
