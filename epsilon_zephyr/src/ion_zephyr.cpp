/*
 * Ion backend for the PicoCalc on Zephyr.
 *
 * Ion is Epsilon's hardware abstraction. Epsilon's shared Ion code and its "dummy"
 * implementations already cover most of it; this file provides what remains: time,
 * keyboard scan, the events loop, the screen, console output and a random source.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ion/console.h>
#include <ion/display.h>
#include <ion/events.h>
#include <ion/keyboard.h>
#include <ion/src/shared/events_modifier.h>
#include <ion/src/shared/init.h>
#include <ion/storage/file_system.h>
#include <ion/src/shared/events.h>
#include <ion/src/shared/keyboard.h>
#include <ion/src/shared/keyboard_queue.h>
#include <ion/timing.h>
#include <stdint.h>

#include <algorithm>

#include "epsilon_glue.h"

static_assert(sizeof(KDColor) == sizeof(uint16_t), "KDColor must be RGB565");

namespace Ion {

/* Same set-up as the desktop simulator: display context, event state, file system. */
void Init() {
  Display::Context::SharedContext.init();
  Events::SharedModifierState.init();
  Events::SharedState.init();
  Storage::FileSystem::sharedFileSystem.init();
}

uint32_t random() { return epsilon_glue_random(); }

namespace Timing {

uint64_t millis() { return epsilon_glue_millis(); }

void msleep(uint32_t ms) { epsilon_glue_msleep(ms); }

}  // namespace Timing

namespace Console {

char readChar() { return 0; }

void writeChar(char c) { epsilon_glue_putchar(c); }

bool clear() { return false; }

void startReception() {}

bool transmissionDone() { return true; }

}  // namespace Console

namespace Display {

/* Epsilon's own display code handles focus/zoom (shared/display.cpp); these are the
 * hardware-space calls it ends up with. */

void pushRectNoFocus(KDRect r, const KDColor* pixels) {
  lcd_push_rect(r.x(), r.y(), r.width(), r.height(),
                reinterpret_cast<const uint16_t*>(pixels));
}

void pushRectUniformNoFocus(KDRect r, KDColor c) {
  lcd_push_uniform(r.x(), r.y(), r.width(), r.height(), c);
}

void pullRectNoFocus(KDRect r, KDColor* pixels) {
  lcd_pull_rect(r.x(), r.y(), r.width(), r.height(),
                reinterpret_cast<uint16_t*>(pixels));
}

void setScreenshotCallback(void (*)(void)) {}

}  // namespace Display

namespace Keyboard {

State scan(bool /*forPython*/) {
  State state(epsilon_glue_scan_keys());
  keyboardWasScanned(state);
  return state;
}

State scanForInterruptionAndPopState() {
  scan();
  return popState();
}

}  // namespace Keyboard

namespace Events {

bool handlePreemption(bool) { return false; }

void setPreemptiveKeyboardState(Keyboard::State) {}

void resetPreemptiveKeyboardState() {}

void didPressNewKey() {}

void setSpinner(bool) {}

Event getPlatformEvent() { return Events::None; }

const char* Event::text() const { return defaultText(); }

Event getEvent(int* timeout) { return sharedGetEvent(timeout); }

size_t copyText(uint8_t eventId, char* buffer, size_t bufferSize) {
  return sharedCopyText(eventId, buffer, bufferSize);
}

bool isDefined(uint8_t eventId) { return sharedIsDefined(eventId); }

void openURL(const char*) {}

bool waitForInterruptingEvent(int maximumDelay, int* timeout) {
  Keyboard::scan();
  /* Key changes do not raise an interrupt here, so the keyboard is polled
   * regularly instead, like the desktop simulator does. */
  constexpr int pollDelay = 10;
  maximumDelay = std::min(pollDelay, maximumDelay);
  if (*timeout < maximumDelay) {
    Timing::msleep(*timeout);
    *timeout = 0;
  } else {
    Timing::msleep(maximumDelay);
    *timeout -= maximumDelay;
  }
  return !Keyboard::Queue::sharedQueue()->isEmpty();
}

}  // namespace Events

}  // namespace Ion
