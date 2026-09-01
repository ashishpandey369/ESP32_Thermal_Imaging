#pragma once

#include <cstddef>
#include <cstdint>

namespace ThermalConfig {

constexpr uint32_t SERIAL_BAUD = 921600;
constexpr uint32_t I2C_FREQUENCY = 400000;
constexpr uint8_t MLX90640_REFRESH_RATE_CODE = 0x03; // 2 Hz during initial bring-up
constexpr float EMISSIVITY = 0.95f;
constexpr float TA_SHIFT = 8.0f;
constexpr std::size_t THERMAL_PIXEL_COUNT = 32 * 24;
constexpr uint16_t WIFI_PORT = 8080;
constexpr char WIFI_AP_SSID[] = "ESP32-Thermal";
constexpr char WIFI_AP_PASSWORD[] = "thermal123";

} // namespace ThermalConfig
