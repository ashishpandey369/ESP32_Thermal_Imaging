import socket

from .frame_protocol import parse_line


class WifiReceiver:
    def __init__(self):
        self.socket = None
        self.buffer = b''
        self.state = {}

    def connect(self, host: str = '192.168.4.1', port: int = 8080):
        self.close()
        self.socket = socket.create_connection((host, port), timeout=3)
        self.socket.setblocking(False)
        self.buffer = b''
        self.state = {}

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

        try:
            chunk = self.socket.recv(65536)
            if chunk:
                self.buffer += chunk
            else:
                self.close()
                return []
        except BlockingIOError:
            pass
        except OSError:
            self.close()
            return []

        frames = []
        while b'\n' in self.buffer:
            raw, self.buffer = self.buffer.split(b'\n', 1)
            try:
                line = raw.decode('ascii')
            except UnicodeDecodeError:
                continue
            frame = parse_line(line, self.state)
            if frame is not None:
                frames.append(frame)
        return frames
