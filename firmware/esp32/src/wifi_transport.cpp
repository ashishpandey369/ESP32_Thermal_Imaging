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

    if (!started) {
        return false;
    }

    server.begin();

    // Run the TCP sender independently from the MLX90640 acquisition loop.
    // A dedicated task prevents a slow Wi-Fi client from stalling capture.
    const BaseType_t taskResult = xTaskCreatePinnedToCore(
        senderTaskEntry,
        "thermal_wifi_tx",
        8192,
        this,
        2,
        &senderTaskHandle,
        0);

    return taskResult == pdPASS;
}

void WifiTransport::sendFrame(const ThermalFrame &frame)
{
    // Latest-frame-only mailbox. The acquisition loop never waits for TCP.
    // If the network is behind, an older unsent frame is simply replaced.
    portENTER_CRITICAL(&frameMux);
    pendingFrame = frame;
    framePending = true;
    portEXIT_CRITICAL(&frameMux);
}

void WifiTransport::senderTaskEntry(void *parameter)
{
    static_cast<WifiTransport *>(parameter)->senderTask();
}

void WifiTransport::senderTask()
{
    uint8_t packet[ThermalProtocol::FRAME_SIZE];

    for (;;) {
        // Accept a new client without involving the sensor loop.
        if (!client || !client.connected()) {
            WiFiClient incoming = server.available();
            if (incoming) {
                client.stop();
                client = incoming;
                client.setNoDelay(true);
            }
        }

        bool haveFrame = false;

        // Snapshot the newest frame into the packet buffer, then release the
        // short critical section before doing CRC calculation or TCP I/O.
        portENTER_CRITICAL(&frameMux);
        if (framePending) {
            std::memset(packet, 0, ThermalProtocol::HEADER_SIZE);

            writeU32LE(packet + 0, ThermalProtocol::MAGIC);
            packet[4] = ThermalProtocol::VERSION;
            packet[5] = 0;
            writeU16LE(packet + 6, ThermalProtocol::HEADER_SIZE);
            writeU32LE(packet + 8, pendingFrame.frameNumber);
            writeU32LE(packet + 12, pendingFrame.timestamp);

            std::memcpy(
                packet + ThermalProtocol::HEADER_SIZE,
                pendingFrame.pixels,
                ThermalProtocol::PIXEL_DATA_SIZE);

            uint8_t *stats = packet + ThermalProtocol::HEADER_SIZE + ThermalProtocol::PIXEL_DATA_SIZE;
            std::memcpy(stats + 0, &pendingFrame.minimum, sizeof(float));
            std::memcpy(stats + 4, &pendingFrame.maximum, sizeof(float));
            std::memcpy(stats + 8, &pendingFrame.average, sizeof(float));
            std::memcpy(stats + 12, &pendingFrame.center, sizeof(float));

            framePending = false;
            haveFrame = true;
        }
        portEXIT_CRITICAL(&frameMux);

        if (!haveFrame) {
            vTaskDelay(pdMS_TO_TICKS(1));
            continue;
        }

        // No client yet: do not retain stale frames. The next sensor frame
        // will replace this one in the mailbox.
        if (!client || !client.connected()) {
            continue;
        }

        const uint32_t crc = ThermalProtocol::crc32(
            packet,
            ThermalProtocol::HEADER_SIZE + ThermalProtocol::PAYLOAD_SIZE);
        writeU32LE(
            packet + ThermalProtocol::HEADER_SIZE + ThermalProtocol::PAYLOAD_SIZE,
            crc);

        const size_t sent = client.write(packet, sizeof(packet));
        if (sent != sizeof(packet)) {
            client.stop();
        }
    }
}
