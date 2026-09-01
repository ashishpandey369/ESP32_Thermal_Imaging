import serial
from serial.tools import list_ports

from .frame_protocol import parse_line


class UsbReceiver:
    def __init__(self):
        self.serial = None
        self.buffer = b''
        self.state = {}

    @staticmethod
    def ports():
        return [port.device for port in list_ports.comports()]

    def connect(self, port: str, baud: int = 921600):
        self.close()
        self.serial = serial.Serial(port, baudrate=baud, timeout=0)
        self.buffer = b''
        self.state = {}

    def close(self):
        if self.serial:
            self.serial.close()
        self.serial = None

    def connected(self):
        return self.serial is not None and self.serial.is_open

    def poll(self):
        if not self.connected():
            return []

        waiting = self.serial.in_waiting
        if waiting:
            self.buffer += self.serial.read(waiting)

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
