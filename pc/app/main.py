import sys

from PySide6.QtCore import QTimer
from PySide6.QtWidgets import (
    QApplication,
    QComboBox,
    QFormLayout,
    QGridLayout,
    QGroupBox,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QMainWindow,
    QPushButton,
    QSpinBox,
    QVBoxLayout,
    QWidget,
)
import pyqtgraph as pg

from communication.usb_receiver import UsbReceiver
from communication.wifi_receiver import WifiReceiver
from thermal.renderer import ThermalRenderer


class ThermalWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle('ESP32 Thermal Imaging')
        self.resize(1200, 760)

        self.usb = UsbReceiver()
        self.wifi = WifiReceiver()
        self.active = None
        self.frame_count = 0
        self.last_frame_number = None

        central = QWidget()
        self.setCentralWidget(central)
        root = QHBoxLayout(central)

        self.image_view = pg.ImageView()
        self.renderer = ThermalRenderer(self.image_view)
        root.addWidget(self.image_view, 1)

        panel = QVBoxLayout()
        root.addLayout(panel)

        connection_box = QGroupBox('Connection')
        connection_form = QFormLayout(connection_box)

        self.port_combo = QComboBox()
        self.refresh_button = QPushButton('Refresh')
        self.usb_button = QPushButton('Connect USB')
        self.usb_button.clicked.connect(self.connect_usb)
        self.refresh_button.clicked.connect(self.refresh_ports)

        port_row = QHBoxLayout()
        port_row.addWidget(self.port_combo, 1)
        port_row.addWidget(self.refresh_button)
        connection_form.addRow('USB port:', port_row)
        connection_form.addRow(self.usb_button)

        self.host_edit = QLineEdit('192.168.4.1')
        self.port_spin = QSpinBox()
        self.port_spin.setRange(1, 65535)
        self.port_spin.setValue(8080)
        self.wifi_button = QPushButton('Connect Wi-Fi')
        self.wifi_button.clicked.connect(self.connect_wifi)
        connection_form.addRow('Wi-Fi host:', self.host_edit)
        connection_form.addRow('TCP port:', self.port_spin)
        connection_form.addRow(self.wifi_button)

        panel.addWidget(connection_box)

        stats_box = QGroupBox('Thermal Data')
        stats = QGridLayout(stats_box)
        self.status_label = QLabel('Disconnected')
        self.frame_label = QLabel('-')
        self.fps_label = QLabel('-')
        self.min_label = QLabel('-')
        self.max_label = QLabel('-')
        self.avg_label = QLabel('-')
        self.center_label = QLabel('-')

        for row, (name, widget) in enumerate([
            ('Status', self.status_label),
            ('Frame', self.frame_label),
            ('FPS', self.fps_label),
            ('Minimum', self.min_label),
            ('Maximum', self.max_label),
            ('Average', self.avg_label),
            ('Center', self.center_label),
        ]):
            stats.addWidget(QLabel(name + ':'), row, 0)
            stats.addWidget(widget, row, 1)

        panel.addWidget(stats_box)
        panel.addStretch()

        self.refresh_ports()

        self.timer = QTimer(self)
        self.timer.timeout.connect(self.poll)
        self.timer.start(20)

    def refresh_ports(self):
        current = self.port_combo.currentText()
        self.port_combo.clear()
        self.port_combo.addItems(self.usb.ports())
        if current:
            index = self.port_combo.findText(current)
            if index >= 0:
                self.port_combo.setCurrentIndex(index)

    def disconnect_all(self):
        self.usb.close()
        self.wifi.close()
        self.active = None

    def connect_usb(self):
        port = self.port_combo.currentText()
        if not port:
            self.status_label.setText('No USB port')
            return
        try:
            self.disconnect_all()
            self.usb.connect(port)
            self.active = self.usb
            self.status_label.setText(f'USB: {port}')
        except Exception as exc:
            self.status_label.setText(f'USB error: {exc}')

    def connect_wifi(self):
        try:
            self.disconnect_all()
            self.wifi.connect(self.host_edit.text().strip(), self.port_spin.value())
            self.active = self.wifi
            self.status_label.setText('Wi-Fi connected')
        except Exception as exc:
            self.status_label.setText(f'Wi-Fi error: {exc}')

    def poll(self):
        if self.active is None:
            return

        try:
            frames = self.active.poll()
        except Exception as exc:
            self.status_label.setText(f'Connection error: {exc}')
            self.disconnect_all()
            return

        for frame in frames:
            self.handle_frame(frame)

    def handle_frame(self, frame):
        self.renderer.update(frame)
        self.frame_label.setText(str(frame['frame']))
        self.min_label.setText(f"{frame['minimum']:.2f} °C")
        self.max_label.setText(f"{frame['maximum']:.2f} °C")
        self.avg_label.setText(f"{frame['average']:.2f} °C")
        self.center_label.setText(f"{frame['center']:.2f} °C")

        if self.last_frame_number is not None:
            delta = frame['frame'] - self.last_frame_number
            if delta > 0:
                self.frame_count += delta
        self.last_frame_number = frame['frame']
        self.fps_label.setText('2 Hz target')

    def closeEvent(self, event):
        self.disconnect_all()
        event.accept()


def main():
    app = QApplication(sys.argv)
    pg.setConfigOptions(imageAxisOrder='row-major')
    window = ThermalWindow()
    window.show()
    sys.exit(app.exec())


if __name__ == '__main__':
    main()
