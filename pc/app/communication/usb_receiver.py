import serial
from serial.tools import list_ports

from .frame_protocol import parse_binary_frames


class UsbReceiver:
    def __init__(self):
        self.serial = None
        self.buffer = b''

    @staticmethod
    def ports():
        return [port.device for port in list_ports.comports()]

    def connect(self, port: str, baud: int = 921600):
        self.close()
        self.serial = serial.Serial(port, baudrate=baud, timeout=0)
        self.buffer = b''

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

        frames, self.buffer = parse_binary_frames(self.buffer)
        # Latest-frame-only keeps the UI live if the PC briefly falls behind.
        return frames[-1:] if frames else []
