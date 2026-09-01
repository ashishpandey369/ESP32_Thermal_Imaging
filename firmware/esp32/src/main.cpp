#include <Arduino.h>
#include <Wire.h>

#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"

namespace {

constexpr uint8_t MLX90640_ADDRESS = 0x33;
constexpr uint8_t SDA_PIN = 21;
constexpr uint8_t SCL_PIN = 22;
constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t I2C_FREQUENCY = 100000; // Start conservatively on ESP32.
constexpr uint8_t REFRESH_RATE_CODE = 0x03; // 2 Hz sensor refresh.
constexpr float EMISSIVITY = 0.95f;
constexpr float TA_SHIFT = 8.0f;

float temperatures[768];
paramsMLX90640 mlx90640;
uint32_t frameNumber = 0;

bool sensorConnected()
{
    Wire.beginTransmission(MLX90640_ADDRESS);
    return Wire.endTransmission() == 0;
}

bool initializeSensor()
{
    Serial.println("[INIT] Checking MLX90640...");

    if (!sensorConnected()) {
        Serial.println("[ERROR] MLX90640 not detected at I2C address 0x33.");
        return false;
    }

    Serial.println("[OK] MLX90640 detected at 0x33.");

    uint16_t eeData[832];
    int status = MLX90640_DumpEE(MLX90640_ADDRESS, eeData);
    if (status != 0) {
        Serial.printf("[ERROR] EEPROM read failed: %d\n", status);
        return false;
    }

    status = MLX90640_ExtractParameters(eeData, &mlx90640);
    if (status != 0) {
        Serial.printf("[ERROR] Parameter extraction failed: %d\n", status);
        return false;
    }

    status = MLX90640_SetRefreshRate(MLX90640_ADDRESS, REFRESH_RATE_CODE);
    if (status != 0) {
        Serial.printf("[WARN] Could not set refresh rate: %d\n", status);
    } else {
        Serial.println("[OK] Sensor refresh rate configured for 2 Hz.");
    }

    return true;
}

bool readThermalFrame()
{
    // The MLX90640 uses two sub-pages to form a complete thermal image.
    // Reading two frames before calculating the image follows the sensor's
    // normal operating model and avoids displaying half-updated data.
    uint16_t frameData[834];

    float minimum = 1000.0f;
    float maximum = -1000.0f;

    for (uint8_t subpage = 0; subpage < 2; ++subpage) {
        int status = MLX90640_GetFrameData(MLX90640_ADDRESS, frameData);
        if (status < 0) {
            Serial.printf("[ERROR] Frame read failed: %d\n", status);
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

    for (size_t i = 0; i < 768; ++i) {
        if (!isfinite(temperatures[i])) {
            Serial.printf("[ERROR] Invalid temperature at pixel %u.\n", static_cast<unsigned>(i));
            return false;
        }

        minimum = min(minimum, temperatures[i]);
        maximum = max(maximum, temperatures[i]);
    }

    const float centerTemperature = temperatures[(12 * 32) + 16];

    // Human-readable diagnostics for the first bring-up milestone.
    Serial.printf(
        "FRAME,%lu,MIN=%.2f,MAX=%.2f,CENTER=%.2f\n",
        static_cast<unsigned long>(frameNumber),
        minimum,
        maximum,
        centerTemperature);

    // Machine-readable 32x24 frame. The PC viewer will consume this later.
    Serial.print("DATA,");
    for (size_t i = 0; i < 768; ++i) {
        Serial.print(temperatures[i], 2);
        if (i != 767) {
            Serial.print(',');
        }
    }
    Serial.println();

    ++frameNumber;
    return true;
}

} // namespace

void setup()
{
    Serial.begin(SERIAL_BAUD);
    delay(1000);

    Serial.println();
    Serial.println("========================================");
    Serial.println(" ESP32 Thermal Imaging Camera");
    Serial.println(" MLX90640 bring-up firmware");
    Serial.println("========================================");

    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(I2C_FREQUENCY);

    Serial.printf("[INIT] I2C SDA: GPIO%d\n", SDA_PIN);
    Serial.printf("[INIT] I2C SCL: GPIO%d\n", SCL_PIN);
    Serial.printf("[INIT] I2C frequency: %lu Hz\n", static_cast<unsigned long>(I2C_FREQUENCY));
    Serial.printf("[INIT] Serial baud: %lu\n", static_cast<unsigned long>(SERIAL_BAUD));

    if (!initializeSensor()) {
        Serial.println("[FATAL] Sensor initialization failed.");
        Serial.println("[FATAL] Check power, GND, SDA, SCL and the I2C address.");
        while (true) {
            delay(1000);
        }
    }

    Serial.println("[READY] Thermal frame acquisition started.");
}

void loop()
{
    const uint32_t start = millis();

    if (!readThermalFrame()) {
        Serial.println("[WARN] Retrying thermal frame acquisition...");
        delay(250);
        return;
    }

    const uint32_t elapsed = millis() - start;
    Serial.printf("TIMING,READ_MS=%lu\n", static_cast<unsigned long>(elapsed));

    // The sensor is configured for 2 Hz, so a small delay prevents the
    // serial output loop from hammering the I2C bus unnecessarily.
    delay(100);
}
