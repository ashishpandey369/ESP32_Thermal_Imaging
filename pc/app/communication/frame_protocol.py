import numpy as np

PIXELS = 32 * 24


def parse_line(line: str, state: dict):
    """Parse one ESP32 CSV protocol line.

    Returns a complete ThermalFrame dict when a DATA line completes a frame,
    otherwise None.
    """
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
