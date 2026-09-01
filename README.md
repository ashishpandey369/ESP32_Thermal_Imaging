# ESP32 Thermal Imaging Camera

A PlatformIO-based thermal imaging project using an ESP32 and an MLX90640 32×24 infrared thermal sensor. The project is designed to turn the ESP32 into a live thermal camera and stream thermal data to a PC for real-time visualization.

## Project Goals

- Read the MLX90640 thermal array reliably using the ESP32.
- Acquire a complete 32×24 thermal frame (768 temperature measurements).
- Stream thermal frames over USB serial during development.
- Build a PC thermal viewer for a live heat-map style image.
- Add interpolation and filtering for a smoother visual display.
- Add temperature measurements such as minimum, maximum, and center temperature.
- Later support Wi-Fi streaming so the camera can operate wirelessly.
- Provide image capture, video recording, CSV logging, and configurable visualization as the project matures.

## Hardware

- ESP32 development board compatible with the PlatformIO `esp32dev` target.
- Grove Thermal Imaging Camera / IR Array based on the MLX90640 32×24 sensor.
- USB cable for power, programming, and initial data streaming.

## System Architecture

```text
          Thermal Scene
                │
                ▼
        ┌────────────────┐
        │    MLX90640    │
        │     32×24      │
        └───────┬────────┘
                │ I²C
                ▼
        ┌────────────────┐
        │      ESP32     │
        │ Frame capture  │
        │ Processing     │
        └───────┬────────┘
                │
         USB Serial / Wi-Fi
                │
                ▼
        ┌────────────────┐
        │       PC       │
        │ Thermal Viewer │
        │ Heat-map       │
        │ Measurements   │
        └────────────────┘
```

## Repository Structure

```text
ESP32_Thermal_Imaging/
├── firmware/
│   └── esp32/
│       ├── src/
│       │   └── main.cpp
│       └── README.md
├── pc/
│   └── README.md
├── hardware/
│   └── wiring/
│       └── README.md
├── documentation/
│   ├── development_log.md
│   ├── communication_protocol.md
│   └── calibration.md
├── images/
├── platformio.ini
└── README.md
```

## Development Roadmap

### Phase 1 — Hardware Bring-Up

1. Confirm ESP32 board and I²C pins. **Done for current `esp32dev` target.**
2. Connect the MLX90640.
3. Detect the sensor on the I²C bus.
4. Read a stable thermal frame.
5. Verify the 768 temperature values.

### Phase 2 — USB Thermal Stream

1. Define a compact serial frame format.
2. Stream thermal data from the ESP32.
3. Add frame sequence numbers and timing information.
4. Validate the stream on the PC.

### Phase 3 — Live PC Thermal Viewer

1. Receive frames over serial.
2. Convert the 32×24 matrix into a display image.
3. Apply interpolation for a smoother visualization.
4. Display minimum, maximum, center, and frame-rate information.

### Phase 4 — Wireless Streaming

1. Add ESP32 Wi-Fi networking.
2. Stream thermal frames over the local network.
3. Maintain the same thermal data protocol where practical.
4. Compare USB and Wi-Fi performance.

### Phase 5 — Advanced Features

- Temperature cursor and hotspot tracking.
- Automatic and manual temperature ranges.
- Image capture.
- Thermal video recording.
- CSV logging.
- Calibration and filtering controls.
- Full-screen PC monitoring interface.

## Development Environment

The firmware is managed with **PlatformIO** and targets the Arduino framework on ESP32. The PC application will be developed separately so the embedded firmware and visualization software remain modular.

## Current Status

**Milestone 1 firmware added — ready for physical ESP32 + MLX90640 bring-up.**

The firmware initializes I²C on GPIO 21/22, checks the MLX90640 at `0x33`, extracts calibration parameters, acquires thermal frames, validates all 768 temperatures, and sends a diagnostic plus machine-readable frame over USB serial at 115200 baud.

## License

License to be selected as the project matures.
