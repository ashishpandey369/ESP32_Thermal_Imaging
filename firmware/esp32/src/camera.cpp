#include "camera.h"
#include "pins.h"

#include <Wire.h>
#include <math.h>

#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"

namespace {

constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t I2C_FREQUENCY = 100000;
constexpr uint8_t REFRESH_RATE_CODE = 0x03; // 2 Hz
constexpr float EMISSIVITY = 0.95f;
constexpr float TA_SHIFT = 8.0f;
constexpr size_t PIXEL_COUNT = 768;

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
    Serial.println("[CAMERA] Checking MLX90640...");

    if (!sensorConnected()) {
        Serial.println("[CAMERA][ERROR] MLX90640 not detected at I2C address 0x33.");
        return false;
    }

    Serial.println("[CAMERA][OK] MLX90640 detected at 0x33.");

    uint16_t eeData[832];
    int status = MLX90640_DumpEE(CAMERA_I2C_ADDRESS, eeData);
    if (status != 0) {
        Serial.printf("[CAMERA][ERROR] EEPROM read failed: %d\n", status);
        return false;
    }

    status = MLX90640_ExtractParameters(eeData, &mlx90640);
    if (status != 0) {
        Serial.printf("[CAMERA][ERROR] Parameter extraction failed: %d\n", status);
        return false;
    }

    status = MLX90640_SetRefreshRate(CAMERA_I2C_ADDRESS, REFRESH_RATE_CODE);
    if (status != 0) {
        Serial.printf("[CAMERA][WARN] Could not set refresh rate: %d\n", status);
    } else {
        Serial.println("[CAMERA][OK] Sensor refresh rate configured for 2 Hz.");
    }

    return true;
}

bool ThermalCamera::begin()
{
    Serial.begin(SERIAL_BAUD);
    delay(1000);

    Serial.println();
    Serial.println("========================================");
    Serial.println(" ESP32 Thermal Imaging Camera");
    Serial.println(" MLX90640 camera module");
    Serial.println("========================================");

    Wire.begin(CAMERA_SDA_PIN, CAMERA_SCL_PIN);
    Wire.setClock(I2C_FREQUENCY);

    Serial.printf("[CAMERA] I2C SDA: GPIO%d\n", CAMERA_SDA_PIN);
    Serial.printf("[CAMERA] I2C SCL: GPIO%d\n", CAMERA_SCL_PIN);
    Serial.printf("[CAMERA] I2C frequency: %lu Hz\n", static_cast<unsigned long>(I2C_FREQUENCY));

    if (!initializeSensor()) {
        Serial.println("[CAMERA][FATAL] Sensor initialization failed.");
        Serial.println("[CAMERA][FATAL] Check power, GND, SDA, SCL and the I2C address.");
        initialized = false;
        return false;
    }

    initialized = true;
    Serial.println("[CAMERA][READY] Thermal frame acquisition started.");
    return true;
}

bool ThermalCamera::readThermalFrame()
{
    uint16_t frameData[834];

    float minimum = 1000.0f;
    float maximum = -1000.0f;

    for (uint8_t subpage = 0; subpage < 2; ++subpage) {
        int status = MLX90640_GetFrameData(CAMERA_I2C_ADDRESS, frameData);
        if (status < 0) {
            Serial.printf("[CAMERA][ERROR] Frame read failed: %d\n", status);
            return false;
        }

        const float ambientTemperature = MLX90640_GetTa(frameData, &mlx90640);
        const float reflectedTemperature = ambientTemperature - TA_SHIFT;

        MLX90640_CalculateTo(
            frameData,
            &mlx90640,
            EMISSIVITY,
            reflectedTemperature,
            temperatures);
    }

    for (size_t i = 0; i < PIXEL_COUNT; ++i) {
        if (!isfinite(temperatures[i])) {
            Serial.printf(
                "[CAMERA][ERROR] Invalid temperature at pixel %u.\n",
                static_cast<unsigned>(i));
            return false;
        }

        minimum = min(minimum, temperatures[i]);
        maximum = max(maximum, temperatures[i]);
    }

    minimumTemperature = minimum;
    maximumTemperature = maximum;
    centerTemperature = temperatures[(12 * 32) + 16];

    Serial.printf(
        "FRAME,%lu,MIN=%.2f,MAX=%.2f,CENTER=%.2f\n",
        static_cast<unsigned long>(frameNumber),
        minimumTemperature,
        maximumTemperature,
        centerTemperature);

    Serial.print("DATA,");
    for (size_t i = 0; i < PIXEL_COUNT; ++i) {
        Serial.print(temperatures[i], 2);
        if (i != PIXEL_COUNT - 1) {
            Serial.print(',');
        }
    }
    Serial.println();

    ++frameNumber;
    return true;
}

bool ThermalCamera::update()
{
    if (!initialized) {
        return false;
    }

    const uint32_t start = millis();

    if (!readThermalFrame()) {
        Serial.println("[CAMERA][WARN] Retrying thermal frame acquisition...");
        delay(250);
        return false;
    }

    const uint32_t elapsed = millis() - start;
    Serial.printf("TIMING,READ_MS=%lu\n", static_cast<unsigned long>(elapsed));

    delay(100);
    return true;
}

const float *ThermalCamera::getTemperatures() const
{
    return temperatures;
}

float ThermalCamera::getMinimumTemperature() const
{
    return minimumTemperature;
}

float ThermalCamera::getMaximumTemperature() const
{
    return maximumTemperature;
}

float ThermalCamera::getCenterTemperature() const
{
    return centerTemperature;
}

uint32_t ThermalCamera::getFrameNumber() const
{
    return frameNumber;
}
