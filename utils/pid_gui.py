#!/usr/bin/env python3

import sys
import socket
import struct

from PyQt6.QtWidgets import (
    QApplication,
    QDoubleSpinBox,
    QWidget,
    QVBoxLayout,
    QHBoxLayout,
    QGridLayout,
    QLabel,
    QSpinBox,
    QPlainTextEdit,
)

from PyQt6.QtCore import Qt, QTimer


# UDP_IP = "192.168.1.221"
UDP_IP = "192.168.0.118"
UDP_PORT = 9999


PID_NAMES = [
    "Pitch",
    "Yaw",
    "BotVel",
    "MotorL",
    "MotorR",
]


class PIDWidget(QWidget):

    def __init__(self, pid_id, sock, log):
        super().__init__()

        self.pid_id = pid_id
        self.sock = sock
        self.log = log

        layout = QGridLayout()

        self.kp = QDoubleSpinBox()
        self.kp.setRange(-550, 500)
        self.kp.setDecimals(7)
        self.kp.valueChanged.connect(self.send)

        self.ki = QDoubleSpinBox()
        self.ki.setRange(-550, 500)
        self.ki.setDecimals(7)
        self.ki.valueChanged.connect(self.send)

        self.kd = QDoubleSpinBox()
        self.kd.setRange(-550, 500)
        self.kd.setDecimals(7)
        self.kd.valueChanged.connect(self.send)

        self.target = QDoubleSpinBox()
        self.target.setRange(-1000, 1000)
        self.target.setDecimals(7)
        self.target.valueChanged.connect(self.send)

        layout.addWidget(QLabel(PID_NAMES[pid_id]), 0, 0)

        layout.addWidget(QLabel("Target"), 1, 0)
        layout.addWidget(self.target, 1, 1)

        layout.addWidget(QLabel("Kp"), 2, 0)
        layout.addWidget(self.kp, 2, 1)

        layout.addWidget(QLabel("Ki"), 3, 0)
        layout.addWidget(self.ki, 3, 1)

        layout.addWidget(QLabel("Kd"), 4, 0)
        layout.addWidget(self.kd, 4, 1)

        self.setLayout(layout)

    def send(self):

        pkt = struct.pack(
            "<Bffff",
            self.pid_id,
            float(self.target.value()),
            float(self.kp.value()),
            float(self.ki.value()),
            float(self.kd.value()),
        )

        self.sock.sendto(pkt, (UDP_IP, UDP_PORT))

        self.log.appendPlainText(
            f"TX -> PID{self.pid_id} "
            f"T={self.target.value()} "
            f"Kp={self.kp.value()} "
            f"Ki={self.ki.value()} "
            f"Kd={self.kd.value()}"
        )


class Window(QWidget):

    def __init__(self):

        super().__init__()

        self.setWindowTitle("Robot PID Tuner")

        # UDP socket
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind(("", 10000))  # 本地端口接收ACK
        self.sock.setblocking(False)

        main_layout = QHBoxLayout()

        # 左侧 PID 控件
        left_layout = QVBoxLayout()

        # 右侧 log
        self.log = QPlainTextEdit()
        self.log.setReadOnly(True)

        self.widgets = []

        for i in range(5):

            w = PIDWidget(i, self.sock, self.log)

            left_layout.addWidget(w)

            self.widgets.append(w)

        main_layout.addLayout(left_layout)
        main_layout.addWidget(self.log)

        self.setLayout(main_layout)

        # 定时检查 ACK
        self.timer = QTimer()
        self.timer.timeout.connect(self.check_ack)
        self.timer.start(50)

    def check_ack(self):

        try:
            data, addr = self.sock.recvfrom(1024)

            try:
                msg = data.decode().strip()
            except:
                msg = data.hex()

            self.log.appendPlainText(
                f"ACK <- {addr[0]}:{addr[1]}  {msg}"
            )

        except BlockingIOError:
            pass


def main():

    app = QApplication(sys.argv)

    win = Window()
    win.resize(700, 500)
    win.show()

    sys.exit(app.exec())


if __name__ == "__main__":
    main()
