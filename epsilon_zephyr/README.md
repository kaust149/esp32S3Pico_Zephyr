# Epsilon for Zephyr (PicoCalc / ESP32-S3)

Zephyr module that builds NumWorks Epsilon from **your own checkout** of the Epsilon
repository, with an Ion backend for the PicoCalc. Epsilon is not included here.

## Files

- `scripts/epsilon_prepare.py` runs Epsilon's host build to generate its sources and writes
  `epsilon_sources.cmake` (source list, include paths, defines).
- `src/ion_zephyr.cpp`, `src/epsilon_glue.[ch]`, `src/epsilon_start.cpp` are the Ion backend.
  They use `lcd_*` (screen) and `keys_*` (keyboard) from the application.
- `CMakeLists.txt`, `Kconfig`, `zephyr/module.yml` make it a Zephyr module (`CONFIG_EPSILON`).

## Use

1. Put this folder at `~/epsilon_zephyr`.
2. Generate the source list (writes `epsilon_sources.cmake` and `generated/`):
   `PATH=~/NumWorks_Epsilon/epsilon-venv/bin:$PATH python scripts/epsilon_prepare.py ~/NumWorks_Epsilon/epsilon --out ~/epsilon_zephyr/epsilon_sources.cmake`
3. Build the application with the module:
   `export EPSILON_MODULE_DIR=$HOME/epsilon_zephyr`
   then your usual `west build ... --pristine`.
