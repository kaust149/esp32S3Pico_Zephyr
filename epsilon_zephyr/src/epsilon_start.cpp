/*
 * Entry point for running Epsilon from the Zephyr application.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ion.h>
#include <ion/src/shared/init.h>

extern "C" void epsilon_start(void) {
  Ion::Init();
  ion_main(0, nullptr);
}
