import struct
import zlib

import numpy as np

PIXELS = 32 * 24
MAGIC = 0x54484D4C
VERSION = 1
HEADER_SIZE = 16
PIXEL_DATA_SIZE = PIXELS * 4
STATS_SIZE = 16
FRAME_SIZE = HEADER_SIZE + PIXEL_DATA_SIZE + STATS_SIZE + 4
MAGIC_BYTES = struct.pack('<I', MAGIC)


def parse_binary_frames(buffer: bytes):
    """Extract complete binary thermal frames from a byte buffer.

    Returns (frames, remaining_buffer). Invalid bytes are discarded until the
    next protocol magic value. If several frames arrive at once, all are
    decoded so the caller can display only the newest one.
    """
    frames = []

    while True:
        start = buffer.find(MAGIC_BYTES)
        if start < 0:
            # Preserve only enough trailing bytes to match a split magic word.
            return frames, buffer[-3:]

        if start:
            buffer = buffer[start:]

        if len(buffer) < FRAME_SIZE:
            return frames, buffer

        header = buffer[:HEADER_SIZE]
        magic, version, _reserved, header_size, frame_number, timestamp = struct.unpack(
            '<IBBHII', header
        )
        if magic != MAGIC or version != VERSION or header_size != HEADER_SIZE:
            buffer = buffer[1:]
            continue

        expected_crc = struct.unpack('<I', buffer[-4:FRAME_SIZE])[0]
        actual_crc = zlib.crc32(buffer[:FRAME_SIZE - 4]) & 0xFFFFFFFF
        if expected_crc != actual_crc:
            # A corrupted stream must not stall forever. Resynchronize at the
            # next possible magic value.
            buffer = buffer[1:]
            continue

        pixel_start = HEADER_SIZE
        pixel_end = pixel_start + PIXEL_DATA_SIZE
        stats = struct.unpack('<4f', buffer[pixel_end:pixel_end + STATS_SIZE])
        pixels = np.frombuffer(
            buffer[pixel_start:pixel_end], dtype='<f4'
        ).copy().reshape((24, 32))

        frames.append({
            'frame': frame_number,
            'timestamp': timestamp,
            'minimum': stats[0],
            'maximum': stats[1],
            'average': stats[2],
            'center': stats[3],
            'pixels': pixels,
        })

        buffer = buffer[FRAME_SIZE:]


def parse_line(line: str, state: dict):
    """Parse the legacy ESP32 CSV protocol line."""
    line = line.strip()
    if not line:
        return None

    parts = line.split(',')

    if parts[0] == 'FRAME' and len(parts) == 7:
        try:
            state['header'] = {
                'frame': int(parts[1]),
                'timestamp': int(parts[2]),
                'minimum': float(parts[3]),
                'maximum': float(parts[4]),
                'average': float(parts[5]),
                'center': float(parts[6]),
            }
        except ValueError:
            state['header'] = None
        return None

    if parts[0] == 'DATA' and len(parts) == PIXELS + 1:
        try:
            values = np.asarray(parts[1:], dtype=np.float32)
        except ValueError:
            return None

        header = state.get('header')
        state['header'] = None
        if header is None or values.size != PIXELS:
            return None

        return {
            **header,
            'pixels': values.reshape((24, 32)),
        }

    return None
