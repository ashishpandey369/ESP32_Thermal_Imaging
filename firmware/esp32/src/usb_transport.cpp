#include "usb_transport.h"
#include "system_config.h"
#include "thermal_protocol.h"

#include <cstring>

namespace {

void writeU16LE(uint8_t *dst, uint16_t value)
{
    dst[0] = static_cast<uint8_t>(value & 0xFFU);
    dst[1] = static_cast<uint8_t>((value >> 8) & 0xFFU);
}

void writeU32LE(uint8_t *dst, uint32_t value)
{
    dst[0] = static_cast<uint8_t>(value & 0xFFU);
    dst[1] = static_cast<uint8_t>((value >> 8) & 0xFFU);
    dst[2] = static_cast<uint8_t>((value >> 16) & 0xFFU);
    dst[3] = static_cast<uint8_t>((value >> 24) & 0xFFU);
}

} // namespace

bool UsbTransport::begin()
{
    Serial.begin(ThermalConfig::SERIAL_BAUD);
    return true;
}

void UsbTransport::sendFrame(const ThermalFrame &frame)
{
    // USB uses the same compact binary protocol as Wi-Fi. This removes the
    // large CSV formatting/printing overhead from the ESP32 frame loop.
    static uint8_t packet[ThermalProtocol::FRAME_SIZE];

    std::memset(packet, 0, ThermalProtocol::HEADER_SIZE);
    writeU32LE(packet + 0, ThermalProtocol::MAGIC);
    packet[4] = ThermalProtocol::VERSION;
    packet[5] = 0;
    writeU16LE(packet + 6, ThermalProtocol::HEADER_SIZE);
    writeU32LE(packet + 8, frame.frameNumber);
    writeU32LE(packet + 12, frame.timestamp);

    std::memcpy(packet + ThermalProtocol::HEADER_SIZE,
                frame.pixels,
                ThermalProtocol::PIXEL_DATA_SIZE);

    uint8_t *stats = packet + ThermalProtocol::HEADER_SIZE + ThermalProtocol::PIXEL_DATA_SIZE;
    std::memcpy(stats + 0, &frame.minimum, sizeof(float));
    std::memcpy(stats + 4, &frame.maximum, sizeof(float));
    std::memcpy(stats + 8, &frame.average, sizeof(float));
    std::memcpy(stats + 12, &frame.center, sizeof(float));

    const uint32_t crc = ThermalProtocol::crc32(
        packet,
        ThermalProtocol::HEADER_SIZE + ThermalProtocol::PAYLOAD_SIZE);
    writeU32LE(packet + ThermalProtocol::HEADER_SIZE + ThermalProtocol::PAYLOAD_SIZE, crc);

    Serial.write(packet, sizeof(packet));
}
