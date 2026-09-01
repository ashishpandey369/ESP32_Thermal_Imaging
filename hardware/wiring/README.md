# Hardware / Wiring

## Current hardware

The thermal sensor used for this project is the Grove Thermal Imaging Camera / IR Array based on the **MLX90640 32×24** sensor.

The module communicates with the ESP32 over I²C and uses the default MLX90640 address `0x33`.

## ESP32 DevKit / `esp32dev` wiring

For the current PlatformIO `esp32dev` target, the firmware uses:

| Thermal camera | ESP32 |
|---|---|
| GND | GND |
| VCC | 3V3 |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

If using the Grove cable, the standard Grove signal convention is:

- Black: GND
- Red: VCC
- White: SDA
- Yellow: SCL

**Power recommendation:** use the ESP32 3.3 V rail for the initial bring-up. The exact module variant should always be checked against its documentation before applying power. The Seeed Grove MLX90640 family is documented around a 3.3 V logic/power design; some versions expose a wider 3.3–5 V input range through their onboard circuitry.

## I²C configuration

- Address: `0x33`
- Initial bus speed: `100 kHz`
- SDA: GPIO 21
- SCL: GPIO 22

We intentionally start at 100 kHz. The SparkFun MLX90640 example notes that higher I²C speeds can be unreliable on some ESP32 setups, so the first milestone prioritizes reliable acquisition. The bus can be optimized after the sensor is stable.

## First bring-up checklist

1. Disconnect USB power before wiring.
2. Connect GND first.
3. Connect VCC to 3V3.
4. Connect SDA to GPIO 21.
5. Connect SCL to GPIO 22.
6. Connect the ESP32 to the PC by USB.
7. Build and upload the PlatformIO firmware.
8. Open the serial monitor at `115200` baud.
9. Confirm the log reports `MLX90640 detected at 0x33`.

## Expected sensor characteristics

The MLX90640 provides a **32×24 = 768-pixel** thermal array. The Grove version is specified for a wide temperature measurement range and supports configurable refresh rates.
