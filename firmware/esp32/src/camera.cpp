#include "camera.h"
#include "pins.h"
#include "system_config.h"

#include <Adafruit_MLX90640.h>
#include <Wire.h>
#include <cmath>

namespace {

Adafruit_MLX90640 mlx90640;

} // namespace

ThermalCamera camera;

bool ThermalCamera::sensorConnected()
{
    Wire.beginTransmission(CAMERA_I2C_ADDRESS);
    return Wire.endTransmission() == 0;
}

bool ThermalCamera::initializeSensor()
{
    if (!sensorConnected()) {
        return false;
    }

    if (!mlx90640.begin(CAMERA_I2C_ADDRESS, &Wire)) {
        return false;
    }

    mlx90640.setMode(MLX90640_CHESS);
    mlx90640.setResolution(MLX90640_ADC_18BIT);
    // 8 Hz gives a noticeably more responsive live view while remaining
    // within the MLX90640 operating range used by this project.
    mlx90640.setRefreshRate(MLX90640_8_HZ);

    return true;
}

bool ThermalCamera::begin()
{
    Wire.begin(CAMERA_SDA_PIN, CAMERA_SCL_PIN);
    Wire.setClock(ThermalConfig::I2C_FREQUENCY);

    initialized = initializeSensor();
    return initialized;
}

bool ThermalCamera::readThermalFrame()
{
    if (mlx90640.getFrame(temperatures) != 0) {
        return false;
    }

    float minimum = INFINITY;
    float maximum = -INFINITY;
    float sum = 0.0f;

    for (std::size_t i = 0; i < ThermalConfig::THERMAL_PIXEL_COUNT; ++i) {
        if (!std::isfinite(temperatures[i])) {
            return false;
        }

        minimum = min(minimum, temperatures[i]);
        maximum = max(maximum, temperatures[i]);
        sum += temperatures[i];
        frame.pixels[i] = temperatures[i];
    }

    frame.frameNumber++;
    frame.timestamp = millis();
    frame.minimum = minimum;
    frame.maximum = maximum;
    frame.average = sum / static_cast<float>(ThermalConfig::THERMAL_PIXEL_COUNT);
    frame.center = temperatures[(12 * 32) + 16];

    return true;
}

bool ThermalCamera::update()
{
    if (!initialized) {
        return false;
    }

    return readThermalFrame();
}

const ThermalFrame &ThermalCamera::getFrame() const
{
    return frame;
}

bool ThermalCamera::isInitialized() const
{
    return initialized;
}
