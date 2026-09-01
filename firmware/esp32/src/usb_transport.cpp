#include "usb_transport.h"
#include "system_config.h"

bool UsbTransport::begin()
{
    Serial.begin(ThermalConfig::SERIAL_BAUD);
    return true;
}

void UsbTransport::sendFrame(const ThermalFrame &frame)
{
    Serial.printf(
        "FRAME,%lu,%lu,%.2f,%.2f,%.2f,%.2f\n",
        static_cast<unsigned long>(frame.frameNumber),
        static_cast<unsigned long>(frame.timestamp),
        frame.minimum,
        frame.maximum,
        frame.average,
        frame.center);

    Serial.print("DATA,");
    for (std::size_t i = 0; i < ThermalConfig::THERMAL_PIXEL_COUNT; ++i) {
        Serial.print(frame.pixels[i], 2);
        if (i + 1 < ThermalConfig::THERMAL_PIXEL_COUNT) {
            Serial.print(',');
        }
    }
    Serial.println();
}
