# Development Log

## 2026-09-01 — Project Initialization

- Created the ESP32 Thermal Imaging project repository.
- Established PlatformIO as the firmware build system.
- Added the initial ESP32 development-board environment.
- Added MLX90640 dependency configuration.
- Added the project architecture and development roadmap.
- Chosen development direction: USB serial thermal streaming first, followed by Wi-Fi streaming.

## 2026-09-01 — Milestone 1 Firmware Added

- Added the first PlatformIO ESP32 firmware at `firmware/esp32/src/main.cpp`.
- Configured I²C for GPIO 21 (SDA) and GPIO 22 (SCL).
- Added MLX90640 address detection at `0x33`.
- Added EEPROM read and calibration-parameter extraction.
- Configured the initial sensor refresh rate for 2 Hz.
- Added complete 32×24 frame acquisition and temperature calculation.
- Added validation for all 768 temperature values.
- Added frame statistics: minimum, maximum, and center temperature.
- Added a simple machine-readable serial frame format for the future PC viewer.
- Documented the initial hardware wiring.

### Current Milestone

**ESP32 + MLX90640 hardware bring-up.**

The next physical test is to connect the thermal camera to the ESP32, upload the firmware, and verify that the serial monitor reports the sensor at `0x33` and produces valid 768-pixel frames.

### Next Step

Build the PC-side Python serial receiver and live thermal visualization after the hardware frame stream is confirmed.
