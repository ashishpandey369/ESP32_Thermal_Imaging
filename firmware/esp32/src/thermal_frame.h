#pragma once

#include <Arduino.h>
#include "system_config.h"

struct ThermalFrame {
    uint32_t frameNumber = 0;
    uint32_t timestamp = 0;

    float pixels[ThermalConfig::THERMAL_PIXEL_COUNT] = {};

    float minimum = 0.0f;
    float maximum = 0.0f;
    float average = 0.0f;
    float center = 0.0f;
};
