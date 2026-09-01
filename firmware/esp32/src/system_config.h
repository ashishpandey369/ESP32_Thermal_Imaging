#pragma once

#include <Arduino.h>

namespace ThermalConfig {

constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t I2C_FREQUENCY = 400000;
constexpr uint8_t MLX90640_REFRESH_RATE_CODE = 0x03; // 2 Hz for initial bring-up
constexpr float EMISSIVITY = 0.95f;
constexpr float TA_SHIFT = 8.0f;
constexpr size_t THERMAL_PIXEL_COUNT = 32 * 24;

} // namespace ThermalConfig
