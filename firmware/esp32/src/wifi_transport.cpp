#include "wifi_transport.h"
#include "system_config.h"
#include "thermal_protocol.h"

#include <cstring>

namespace {

void writeU16LE(uint8_t *dst, uint16_t value)
{
    dst[0] = static_cast<uint8_t>(value & 0xFFu);
    dst[1] = static_cast<uint8_t>((value >> 8) & 0xFFu);
}

void writeU32LE(uint8_t *dst, uint32_t value)
{
    dst[0] = static_cast<uint8_t>(value & 0xFFu);
    dst[1] = static_cast<uint8_t>((value >> 8) & 0xFFu);
    dst[2] = static_cast<uint8_t>((value >> 16) & 0xFFu);
    dst[3] = static_cast<uint8_t>((value >> 24) & 0xFFu);
}

} // namespace

bool WifiTransport::begin()
{
    WiFi.mode(WIFI_AP);
    WiFi.setSleep(false); // Reduce Wi-Fi power-save latency for the live stream.

    const bool started = WiFi.softAP(
        ThermalConfig::WIFI_AP_SSID,
        ThermalConfig::WIFI_AP_PASSWORD,
        6,
        false,
        1);

    if (started) {
        server.begin();
    }

    return started;
}

void WifiTransport::sendFrame(const ThermalFrame &frame)
{
    if (!client || !client.connected()) {
        WiFiClient incoming = server.available();
        if (incoming) {
            client.stop();
            client = incoming;
            client.setNoDelay(true);
        }
    }

    if (!client || !client.connected()) {
        return;
    }

    // Binary frame: 16-byte header + 3072-byte float32 pixel payload
    // + 16 bytes of statistics + 4-byte CRC32 = 3108 bytes.
    uint8_t packet[ThermalProtocol::FRAME_SIZE];
    std::memset(packet, 0, ThermalProtocol::HEADER_SIZE);

    writeU32LE(packet + 0, ThermalProtocol::MAGIC);
    packet[4] = ThermalProtocol::VERSION;
    packet[5] = 0;
    writeU16LE(packet + 6, ThermalProtocol::HEADER_SIZE);
    writeU32LE(packet + 8, frame.frameNumber);
    writeU32LE(packet + 12, frame.timestamp);

    std::memcpy(
        packet + ThermalProtocol::HEADER_SIZE,
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

    const size_t sent = client.write(packet, sizeof(packet));
    if (sent != sizeof(packet)) {
        client.stop();
    }
}
