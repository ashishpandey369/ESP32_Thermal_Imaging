import socket

from .frame_protocol import parse_binary_frames


class WifiReceiver:
    def __init__(self):
        self.socket = None
        self.buffer = b''

    def connect(self, host: str = '192.168.4.1', port: int = 8080):
        self.close()
        self.socket = socket.create_connection((host, port), timeout=3)
        self.socket.setblocking(False)
        self.buffer = b''

    def close(self):
        if self.socket:
            try:
                self.socket.close()
            except OSError:
                pass
        self.socket = None

    def connected(self):
        return self.socket is not None

    def poll(self):
        if not self.connected():
            return []

        # Drain all currently available bytes. We intentionally display only
        # the newest complete frame so network buffering cannot become visible
        # latency as the sensor rate increases.
        while True:
            try:
                chunk = self.socket.recv(65536)
                if not chunk:
                    self.close()
                    return []
                self.buffer += chunk
                if len(chunk) < 65536:
                    break
            except BlockingIOError:
                break
            except OSError:
                self.close()
                return []

        frames, self.buffer = parse_binary_frames(self.buffer)
        return frames[-1:] if frames else []
