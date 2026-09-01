# ESP32 Firmware

PlatformIO firmware for the thermal camera controller.

## Milestone 1 — MLX90640 bring-up

`src/main.cpp` currently performs the first hardware validation stage:

1. Starts the ESP32 I²C interface on GPIO 21/22.
2. Checks for the MLX90640 at address `0x33`.
3. Reads the sensor EEPROM calibration data.
4. Extracts the MLX90640 calibration parameters.
5. Configures a conservative 2 Hz refresh rate.
6. Reads two sensor sub-pages for each complete thermal frame.
7. Calculates all 768 temperatures.
8. Validates that the temperature values are finite.
9. Prints frame statistics and a machine-readable `DATA,` line over USB serial.

## Serial output

Serial speed: `115200` baud.

A successful startup should include:

```text
[OK] MLX90640 detected at 0x33.
[OK] Sensor refresh rate configured for 2 Hz.
[READY] Thermal frame acquisition started.
```

Each acquired frame contains:

```text
FRAME,<sequence>,MIN=<temperature>,MAX=<temperature>,CENTER=<temperature>
DATA,<768 comma-separated temperature values>
TIMING,READ_MS=<milliseconds>
```

The `DATA` line is intentionally simple for the first milestone. The PC application will parse it and later the transport can be upgraded to a compact binary protocol for higher frame rates.

## Build

From the repository root:

```bash
pio run
```

Upload:

```bash
pio run --target upload
```

Open the serial monitor:

```bash
pio device monitor -b 115200
```

## Current limitation

The firmware assumes the PlatformIO `esp32dev` target and therefore uses GPIO 21 for SDA and GPIO 22 for SCL. If a different ESP32 board is selected later, the pin configuration should be updated explicitly rather than hidden inside the application logic.
