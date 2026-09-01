#include "thermal_protocol.h"

namespace ThermalProtocol {

uint32_t crc32(const uint8_t* data, std::size_t length)
{
    uint32_t crc = 0xFFFFFFFFu;

    for (std::size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; ++bit) {
            const uint32_t mask = -(crc & 1u);
            crc = (crc >> 1) ^ (0xEDB88320u & mask);
        }
    }

    return ~crc;
}

} // namespace ThermalProtocol
