#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include "system_config.h"
#include "thermal_transport.h"

class WifiTransport final : public ThermalTransport
{
public:
    bool begin() override;
    void sendFrame(const ThermalFrame &frame) override;

private:
    static void senderTaskEntry(void *parameter);
    void senderTask();

    WiFiServer server{ThermalConfig::WIFI_PORT};
    WiFiClient client;

    // The acquisition loop only replaces this latest frame. The Wi-Fi task
    // owns the socket and can therefore block on TCP without blocking the
    // MLX90640 acquisition loop.
    ThermalFrame pendingFrame{};
    bool framePending = false;
    portMUX_TYPE frameMux = portMUX_INITIALIZER_UNLOCKED;
    TaskHandle_t senderTaskHandle = nullptr;
};
