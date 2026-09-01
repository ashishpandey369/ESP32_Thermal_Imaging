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
    WiFiServer server{ThermalConfig::WIFI_PORT};
    WiFiClient client;
};
