#pragma once

#include <Arduino.h>
#include "thermal_transport.h"

class UsbTransport final : public ThermalTransport
{
public:
    bool begin() override;
    void sendFrame(const ThermalFrame &frame) override;
};
