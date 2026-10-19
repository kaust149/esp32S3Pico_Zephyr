# NumWorks Epsilon on a PicoCalc (ESP32-S3 + Zephyr)

This runs the full NumWorks Epsilon calculator OS on a Clockwork PicoCalc, driven by a
Waveshare ESP32-S3-Pico running Zephyr. It boots to the Epsilon home screen and takes
input from the PicoCalc keyboard.

Status: first working build. Calculation, menus and the keyboard work. Storage does not
persist yet, and the internal RAM is almost full (see Limits).

Epsilon itself is **not** in this repository. NumWorks keeps all rights to it. You fetch
it yourself and the build uses your copy.

## Hardware

- PicoCalc (ILI9488 320x320 display, keyboard behind an I2C "southbridge" at 0x1F)
- Waveshare ESP32-S3-Pico: 16 MB flash, 2 MB PSRAM
- USB cable to the board's native USB port (console and flashing at /dev/ttyACM0)

## What is in the repository

- `samples/hello_world/`: the application. It sets up the display (`lcd.c`), the keyboard
  (`keys*.c`, `numworks_keys.h`) and starts Epsilon (`main.c`).
- `epsilon_zephyr/`: a Zephyr module with the Ion backend (`src/ion_zephyr.cpp`,
  `src/epsilon_glue.c`, `src/epsilon_start.cpp`) and `scripts/epsilon_prepare.py`, which
  generates the Epsilon source list.
- `boards/espressif/esp32s3_devkitc/`: board overlay (display, I2C, console on USB).

## 1. Prerequisites (Ubuntu)

1. Install Zephyr with `west` and its SDK, following the Zephyr "Getting Started" guide.
   Use the Python venv it creates (`~/zephyrproject/.venv`).
2. Clone this repository as your Zephyr tree (branch `ZSephyr_With_NumWorks`).
3. Install host packages for Epsilon's code generation: `build-essential`, `git`,
   `python3-venv`, and development packages for libpng and libjpeg. A few simulator-only
   targets may fail in the host build, and the script tolerates that.
4. At least 8 GB RAM for the build. If the compiler crashes with a segmentation fault,
   build with fewer jobs (`-j2`) or add swap.

## 2. Get Epsilon

```
mkdir -p ~/NumWorks_Epsilon && cd ~/NumWorks_Epsilon
git clone https://github.com/numworks/epsilon.git
cd epsilon && git checkout ea4d93b       # the version this port was made against
cd ..
python3 -m venv epsilon-venv
epsilon-venv/bin/pip install lz4 pypng stringcase
```

## 3. Generate the Epsilon source list

This runs Epsilon's host build once to produce its generated files (translations, fonts,
icons, MicroPython tables) and then writes `epsilon_zephyr/epsilon_sources.cmake`:

```
cd epsilon_zephyr      # the folder inside this repository
PATH=~/NumWorks_Epsilon/epsilon-venv/bin:$PATH \
  python scripts/epsilon_prepare.py ~/NumWorks_Epsilon/epsilon --out $PWD/epsilon_sources.cmake
```

The generated file holds paths for your machine and is not committed to git.
Redo this step if you move the Epsilon checkout.

## 4. Build and flash

In every new terminal first:

```
source ~/zephyrproject/.venv/bin/activate
export EPSILON_MODULE_DIR=<path to this repo>/epsilon_zephyr
cd <path to this repo>
```

The application settings are in `samples/hello_world/prj.conf` (C++23, libc++, 28 KB main
stack, `CONFIG_ESP_SPIRAM_HEAP_SIZE=0`). First build, with MCUboot through sysbuild:

```
west build -b esp32s3_devkitc/esp32s3/procpu -S flash-16M -S psram-2M \
  --sysbuild samples/hello_world --pristine
west flash --esp-device /dev/ttyACM0 --esp-baud-rate 115200
```

Later builds can drop `-b`, `-S` and `--pristine`:
`west build --sysbuild samples/hello_world`.

A serial monitor on `/dev/ttyACM0` shows the log. Key events are printed there while
`KEYS_DEBUG_EVENTS` is 1 in `samples/hello_world/src/keys.c`.

## Using it

| PicoCalc key | NumWorks key |
|---|---|
| Arrow keys | Arrows |
| Enter | OK |
| Esc | Back |
| Home or F1 | Home |
| Backspace, Del | Backspace |
| Alt | Alpha |
| Shift (tap, then next key) | Shift |
| `0-9 . , + - * / ( ) ^` | the same keys |
| `=` | EXE |
| `s c t` | sin, cos, tan |
| `l g e` | ln, log, exp |
| `p r q` | pi, square root, x squared |
| `i x v b` | i, x/n/t, Var, Toolbox |
| `a n` | Ans, EE |

Symbols typed with the PicoCalc's Shift (`+ ( ) * ^`) work directly. Tap Shift on its own
for NumWorks' second functions, then press the key.

## How it works

- Epsilon is compiled freestanding as a Zephyr module, using the same flags as its own
  build (`-std=c++23 -fno-exceptions -fno-rtti`). Its hardware layer, Ion, is replaced by
  `ion_zephyr.cpp`: timing, keyboard scan, console, and display push/pull.
- The display layer draws Epsilon's 320x240 RGB565 image from a shadow framebuffer in
  PSRAM into the ILI9488, with the vertical offset to centre it.
- The keyboard layer maps PicoCalc southbridge events onto NumWorks' 46-key bitmap.

## Limits and known issues

- Internal RAM (dram0) is about 99% used. Epsilon's static data (about 218 KB) still
  sits in internal RAM, and only a few KB remain for runtime. Moving it to PSRAM is open
  work, blocked by the way the ESP32-S3 linker script shares an address window between
  flash rodata and PSRAM.
- Storage does not persist across reboots yet.
- Colours and speed need more checking. The SPI clock and wire format are still being tuned.
- setjmp/longjmp (used by MicroPython and Epsilon's circuit breaker) is not fully validated on Xtensa.

## Licence

The files here are released under the licence in this repository. Epsilon is separate
and is under NumWorks' own licence, so check it before redistributing any built binary.
