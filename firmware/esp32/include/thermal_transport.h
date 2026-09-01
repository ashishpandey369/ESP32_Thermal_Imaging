#pragma once

#include "thermal_frame.h"

class ThermalTransport
{
public:
    virtual ~ThermalTransport() = default;
    virtual bool begin() = 0;
    virtual void sendFrame(const ThermalFrame &frame) = 0;
};
