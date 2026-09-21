# MicroPython Utilities

This directory contains small MicroPython test utilities for ESP32 hardware bring-up.

## Solar voltage test

solar_voltage_test.py reads a solar-panel voltage divider connected to GPIO32.

### Configuration

- ADC pin: GPIO32
- ADC attenuation: ADC.ATTN_11DB
- Samples per reading: 10
- Sample interval: 5 ms
- Output interval: 1 second
- Calibration slope: 0.000854
- Calibration offset: 0.079

### Running

Open solar_voltage_test.py in Thonny and run it on the ESP32 with MicroPython installed.

The serial output shows the averaged raw ADC value and calculated solar-panel voltage.

This utility is intentionally kept separate from the main PlatformIO/Arduino thermal-imaging firmware. It can be used for hardware validation without changing the MLX90640 acquisition or binary thermal-stream protocol.
