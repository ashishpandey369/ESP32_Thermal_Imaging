# Hardware / Wiring

## MLX90640 to ESP32

The MLX90640 communicates over I²C. Use the exact GPIO mapping for the ESP32 board being used and document it here before wiring the final build.

Typical development wiring will include:

- VIN / appropriate sensor supply
- GND
- SDA
- SCL

**Do not assume pin voltage or pull-up requirements from the module photo alone. Verify the exact module variant and ESP32 board before applying power.**

The sensor is expected to use the standard MLX90640 I²C address `0x33` unless the module configuration indicates otherwise.
