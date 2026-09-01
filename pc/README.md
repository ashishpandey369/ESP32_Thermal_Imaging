# PC Thermal Viewer

Desktop application for viewing the ESP32 + MLX90640 thermal camera as a live image.

## Phase 3 scope

- USB serial live thermal stream
- Wi-Fi TCP live thermal stream
- One common frame protocol for both transports
- Live 32 x 24 thermal image rendering
- Minimum, maximum, average, center temperature and frame information

## Software stack

- Python
- PySide6
- NumPy
- pyqtgraph
- pyserial

## Run

From the `pc` directory:

```text
python -m venv .venv
.venv\Scripts\activate
pip install -r requirements.txt
python -m app.main
```

For Wi-Fi mode, connect the laptop to the ESP32 access point:

- SSID: `ESP32-Thermal`
- Password: `thermal123`
- TCP server: `192.168.4.1:8080`

For USB mode, select the ESP32 COM port and connect at 921600 baud.
