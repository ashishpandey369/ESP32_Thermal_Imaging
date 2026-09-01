#include "camera.h"
#include "pins.h"

#include <Wire.h>
#include <math.h>

#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"

namespace {

paramsMLX90640 mlx90640;

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

    uint16_t eeData[832];
    int status = MLX90640_DumpEE(CAMERA_I2C_ADDRESS, eeData);
    if (status != 0) {
        return false;
    }

    status = MLX90640_ExtractParameters(eeData, &mlx90640);
    if (status != 0) {
        return false;
    }

    status = MLX90640_SetRefreshRate(
        CAMERA_I2C_ADDRESS,
        ThermalConfig::MLX90640_REFRESH_RATE_CODE);

    return status == 0;
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
    uint16_t frameData[834];

    float minimum = 1000.0f;
    float maximum = -1000.0f;
    float sum = 0.0f;

    for (uint8_t subpage = 0; subpage < 2; ++subpage) {
        const int status = MLX90640_GetFrameData(CAMERA_I2C_ADDRESS, frameData);
        if (status < 0) {
            return false;
        }

        const float ambientTemperature = MLX90640_GetTa(frameData, &mlx90640);
        const float reflectedTemperature = ambientTemperature - ThermalConfig::TA_SHIFT;

        MLX90640_CalculateTo(
            frameData,
            &mlx90640,
            ThermalConfig::EMISSIVITY,
            reflectedTemperature,
            temperatures);
    }

    for (std::size_t i = 0; i < ThermalConfig::THERMAL_PIXEL_COUNT; ++i) {
        if (!isfinite(temperatures[i])) {
            return false;
        }

        minimum = min(minimum, temperatures[i]);
        maximum = max(maximum, temperatures[i]);
        sum += temperatures[i];
    }

    frame.frameNumber++;
    frame.timestamp = millis();
    frame.minimum = minimum;
    frame.maximum = maximum;
    frame.average = sum / static_cast<float>(ThermalConfig::THERMAL_PIXEL_COUNT);
    frame.center = temperatures[(12 * 32) + 16];

    for (std::size_t i = 0; i < ThermalConfig::THERMAL_PIXEL_COUNT; ++i) {
        frame.pixels[i] = temperatures[i];
    }

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
