#pragma once

#include <cstddef>
#include <cstdint>

namespace ThermalProtocol {

constexpr uint32_t MAGIC = 0x54484D4C; // "THML"
constexpr uint8_t VERSION = 1;
constexpr uint16_t HEADER_SIZE = 16;
constexpr uint16_t PIXEL_COUNT = 768;
constexpr uint16_t PIXEL_DATA_SIZE = PIXEL_COUNT * sizeof(float);
constexpr uint16_t STATS_SIZE = 4 * sizeof(float);
constexpr uint16_t PAYLOAD_SIZE = PIXEL_DATA_SIZE + STATS_SIZE;
constexpr uint16_t FRAME_SIZE = HEADER_SIZE + PAYLOAD_SIZE + sizeof(uint32_t);

// Wire format (little-endian):
// Header: magic[4], version[1], reserved[1], header_size[2], frame_number[4], timestamp[4]
// Payload: 768 float32 temperatures, min, max, average, center
// Trailer: CRC32 over header + payload

uint32_t crc32(const uint8_t* data, std::size_t length);

} // namespace ThermalProtocol
