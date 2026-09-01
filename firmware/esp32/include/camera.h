#pragma once

#include <Arduino.h>
#include "thermal_frame.h"

class ThermalCamera
{
public:
    bool begin();
    bool update();

    const ThermalFrame &getFrame() const;
    bool isInitialized() const;

private:
    bool sensorConnected();
    bool initializeSensor();
    bool readThermalFrame();

    ThermalFrame frame{};
    float temperatures[ThermalConfig::THERMAL_PIXEL_COUNT] = {};
    bool initialized = false;
};

extern ThermalCamera camera;
