#pragma once

#include <Arduino.h>

class ThermalCamera
{
public:
    bool begin();
    bool update();

    const float *getTemperatures() const;
    float getMinimumTemperature() const;
    float getMaximumTemperature() const;
    float getCenterTemperature() const;
    uint32_t getFrameNumber() const;

private:
    bool sensorConnected();
    bool initializeSensor();
    bool readThermalFrame();

    float temperatures[768] = {};
    uint32_t frameNumber = 0;
    float minimumTemperature = 0.0f;
    float maximumTemperature = 0.0f;
    float centerTemperature = 0.0f;
    bool initialized = false;
};

extern ThermalCamera camera;
