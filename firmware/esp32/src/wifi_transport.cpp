#include "wifi_transport.h"
#include "system_config.h"

bool WifiTransport::begin()
{
    WiFi.mode(WIFI_AP);

    const bool started = WiFi.softAP(
        ThermalConfig::WIFI_AP_SSID,
        ThermalConfig::WIFI_AP_PASSWORD);

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
        }
    }

    if (!client || !client.connected()) {
        return;
    }

    client.printf(
        "FRAME,%lu,%lu,%.2f,%.2f,%.2f,%.2f\n",
        static_cast<unsigned long>(frame.frameNumber),
        static_cast<unsigned long>(frame.timestamp),
        frame.minimum,
        frame.maximum,
        frame.average,
        frame.center);

    client.print("DATA,");
    for (std::size_t i = 0; i < ThermalConfig::THERMAL_PIXEL_COUNT; ++i) {
        client.print(frame.pixels[i], 2);
        if (i + 1 < ThermalConfig::THERMAL_PIXEL_COUNT) {
            client.print(',');
        }
    }
    client.println();
}
