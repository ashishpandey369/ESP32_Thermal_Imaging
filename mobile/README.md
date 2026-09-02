# ESP32 Thermal Imaging — Android App

Flutter Android client for the ESP32 MLX90640 thermal camera.

## Live Wi-Fi connection

1. Power the ESP32 with the MLX90640 connected.
2. Join the phone to Wi-Fi network `ESP32-Thermal`.
3. Password: `thermal123`.
4. In the app use host `192.168.4.1` and TCP port `8080`.
5. Tap **Connect**.

The app receives the current ESP32 CSV thermal stream and renders the 32×24 MLX90640 array with an Inferno-like thermal map and live statistics.

## Build APK

From the repository root, after Flutter is installed:

```powershell
cd mobile
flutter create .
flutter pub get
flutter build apk --release
```

The generated APK will be under `build/app/outputs/flutter-apk/`.
