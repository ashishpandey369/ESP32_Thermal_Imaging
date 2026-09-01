# Thermal Frame Communication Protocol

This document defines the data transport between the ESP32 firmware and the PC thermal viewer.

## Initial Transport

USB serial at `115200` baud for development and debugging.

## Planned Frame Structure

Each thermal frame contains:

- Frame identifier / sequence number
- Sensor dimensions: `32×24`
- 768 temperature samples
- Optional frame timestamp
- Optional minimum / maximum temperature metadata

The first implementation may use a human-readable diagnostic format to simplify bring-up. Once sensor acquisition is proven stable, the stream can move to a compact binary framing format for higher frame rates and lower serial overhead.

## Design Requirement

The PC viewer must be able to validate frame boundaries and reject incomplete or malformed frames rather than displaying corrupted thermal data.
