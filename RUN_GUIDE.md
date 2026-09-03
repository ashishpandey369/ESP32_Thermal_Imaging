# ESP32 Thermal Imaging — Quick Run Guide

This guide contains the commands needed to build, upload, and run the ESP32 firmware and the PC application.

## 1. Update the local repository

From the repository root:

```powershell
git pull
```

## 2. Build and upload ESP32 firmware

Make sure the ESP32 is connected by USB.

Build:

```powershell
pio run
```

Upload:

```powershell
pio run -t upload
```

Or build and upload in one command:

```powershell
pio run -t upload
```

## 3. Open ESP32 serial monitor

Use this in a separate terminal:

```powershell
pio device monitor
```

The configured baud rate is **921600**.

If PlatformIO cannot find the correct COM port, list available devices with:

```powershell
pio device list
```

## 4. Run the PC application

Run this command from the **repository root**:

```powershell
.\pc\.venv\Scripts\python.exe .\pc\app\main.py
```

If the terminal is currently inside another repository folder such as `mobile`, first return to the root:

```powershell
cd ..
```

Then run:

```powershell
.\pc\.venv\Scripts\python.exe .\pc\app\main.py
```

## 5. Wi-Fi connection

The ESP32 currently creates its own Wi-Fi access point.

| Setting | Value |
|---|---|
| SSID | `ESP32-Thermal` |
| Password | `thermal123` |
| ESP32 IP | `192.168.4.1` |
| TCP port | `8080` |
| Thermal refresh target | `16 Hz` |

Connect the PC or phone directly to **ESP32-Thermal** before starting the Wi-Fi stream.

## 6. PC application

After launching the PC application:

1. Select the **Wi-Fi** connection mode.
2. Enter:
   - IP: `192.168.4.1`
   - Port: `8080`
3. Connect.
4. The live thermal image should start updating.

## 7. USB connection

For USB streaming, keep the ESP32 connected to the PC with the USB cable.

The firmware uses the same binary thermal frame protocol for USB and Wi-Fi.

The USB serial speed is:

```text
921600 baud
```

## 8. Flutter Android application

Go to the mobile project:

```powershell
cd mobile
```

Get dependencies:

```powershell
flutter pub get
```

Run on a connected Android device:

```powershell
flutter run
```

Build a release APK:

```powershell
flutter build apk --release
```

The release APK will be generated under:

```text
mobile\build\app\outputs\flutter-apk\app-release.apk
```

## 9. Android Wi-Fi

Connect the Android phone to:

```text
ESP32-Thermal
```

Then use:

```text
IP:   192.168.4.1
Port: 8080
```

## 10. Recommended test procedure

For performance testing, use this order:

1. Connect the MLX90640 to the ESP32.
2. Connect ESP32 to the PC by USB.
3. Upload the latest firmware.
4. Connect the PC/phone to `ESP32-Thermal`.
5. Start the PC application or Android application.
6. Test Wi-Fi streaming at **16 Hz**.
7. Compare Wi-Fi responsiveness with USB.
8. Check the displayed FPS and dropped-frame count.

Do **not** move to 32 Hz until the 16 Hz Wi-Fi pipeline is confirmed to be responsive.

## 11. Quick command list

### ESP32

```powershell
# Update
git pull

# Build + upload
pio run -t upload

# Serial monitor
pio device monitor
```

### PC application

```powershell
.\pc\.venv\Scripts\python.exe .\pc\app\main.py
```

### Flutter

```powershell
cd mobile
flutter pub get
flutter run
```

### Release APK

```powershell
cd mobile
flutter build apk --release
```

## 12. Project architecture

```text
MLX90640 (32×24)
        │
        ▼
      ESP32
        │
        ├── USB ──► PC
        │
        └── Wi-Fi ──► PC / Android

ESP32 sends raw thermal measurements.
PC/Android performs display interpolation and visualization.
```

The ESP32 is intentionally kept focused on **sensor acquisition and transport** rather than expensive display processing. This leaves the PC and phone CPU/GPU available for smooth rendering.
