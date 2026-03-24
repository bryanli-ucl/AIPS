import math
import socket
import struct
import sys
import threading
import time
from select import select

from PySide6.QtCore import Qt, QTimer, Signal, QObject
from PySide6.QtGui import QColor, QFont, QPainter, QPen, QBrush
from PySide6.QtWidgets import (
    QApplication, QFrame, QGridLayout, QHBoxLayout, QLabel, QMainWindow,
    QPushButton, QPlainTextEdit, QSizePolicy, QSlider, QVBoxLayout, QWidget
)

# =========================
# Network / Control Config
# =========================
MCU_IP = "172.20.10.6"
MCU_PORT = 9999
SEND_INTERVAL = 0.1  # seconds

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("", 0))
sock.settimeout(1.0)

speed = 100.0
pressed_keys = set()
pressed_lock = threading.Lock()
speed_lock = threading.Lock()

KEY_VECTORS = {
    "w": (0.0,  1.0),
    "s": (0.0, -1.0),
    "a": (-1.0, 0.0),
    "d": (1.0,  0.0),
}


def compute_vector():
    with pressed_lock:
        keys = set(pressed_keys)

    if not keys:
        return 0.0, 0.0

    dx = sum(KEY_VECTORS[k][0] for k in keys)
    dy = sum(KEY_VECTORS[k][1] for k in keys)
    length = math.sqrt(dx * dx + dy * dy)

    if length == 0:
        return 0.0, 0.0

    with speed_lock:
        s = speed

    return dx / length * s, dy / length * s


# =========================
# Signals bridge
# =========================
class Bridge(QObject):
    rx_log = Signal(str)
    status = Signal(float, float, bytes)


bridge = Bridge()


# =========================
# Threads
# =========================
def receive_loop():
    while True:
        try:
            ready, _, _ = select([sock], [], [], 0.1)
            if ready:
                data, addr = sock.recvfrom(1024)
                try:
                    txt = data.decode()
                    if txt != "OK":
                        bridge.rx_log.emit(f"[RX] {addr[0]} → {txt}")
                except Exception:
                    bridge.rx_log.emit(f"[RX] {addr[0]} → {data.hex()}")
        except OSError:
            break
        except Exception:
            pass


def send_loop():
    while True:
        try:
            vx, vy = compute_vector()
            payload = struct.pack("<ff", vx, vy)

            try:
                sock.sendto(payload, (MCU_IP, MCU_PORT))
            except Exception:
                pass

            bridge.status.emit(vx, vy, payload)
            time.sleep(SEND_INTERVAL)
        except OSError:
            break
        except Exception:
            pass


# =========================
# Custom Widgets
# =========================
class HeaderButton(QPushButton):
    def __init__(self, text):
        super().__init__(text)
        self.setCursor(Qt.PointingHandCursor)
        self.setObjectName("HeaderButton")
        self.setFlat(True)


class NewsCard(QFrame):
    def __init__(self, title, tag=None):
        super().__init__()
        self.setObjectName("NewsCard")
        self.setMinimumHeight(110)

        layout = QHBoxLayout(self)
        layout.setContentsMargins(12, 12, 12, 12)
        layout.setSpacing(12)

        thumb = QFrame()
        thumb.setFixedSize(110, 78)
        thumb.setObjectName("Thumb")
        layout.addWidget(thumb)

        text_col = QVBoxLayout()
        text_col.setContentsMargins(0, 0, 0, 0)
        text_col.setSpacing(6)

        if tag:
            tag_label = QLabel(tag)
            tag_label.setObjectName("TagLabel")
            tag_label.setFixedWidth(90)
            text_col.addWidget(tag_label, alignment=Qt.AlignLeft)

        title_label = QLabel(title)
        title_label.setWordWrap(True)
        title_label.setObjectName("SideTitle")
        text_col.addWidget(title_label)
        text_col.addStretch()

        layout.addLayout(text_col)


class KeyBox(QLabel):
    def __init__(self, text, key_id):
        super().__init__(text)
        self.key_id = key_id
        self.active = False
        self.setAlignment(Qt.AlignCenter)
        self.setFixedSize(72, 58)
        self.setObjectName("KeyBox")
        self.update_style()

    def set_active(self, active: bool):
        self.active = active
        self.update_style()

    def update_style(self):
        if self.active:
            self.setStyleSheet("""
                QLabel {
                    background-color: #ff2a2a;
                    color: #ffffff;
                    border: 1px solid #ff4d4d;
                    border-radius: 12px;
                    font-size: 22px;
                    font-weight: 800;
                }
            """)
        else:
            self.setStyleSheet("""
                QLabel {
                    background-color: #171b28;
                    color: #7f879d;
                    border: 1px solid #252b3d;
                    border-radius: 12px;
                    font-size: 22px;
                    font-weight: 800;
                }
            """)


class VectorCanvas(QWidget):
    def __init__(self):
        super().__init__()
        self.setMinimumSize(180, 180)
        self.vx = 0.0
        self.vy = 0.0
        self.max_speed = 100.0

    def set_vector(self, vx, vy, max_speed):
        self.vx = vx
        self.vy = vy
        self.max_speed = max_speed if max_speed > 0 else 1.0
        self.update()

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)

        rect = self.rect()
        painter.fillRect(rect, QColor("#11141d"))

        margin = 14
        cx = rect.center().x()
        cy = rect.center().y()
        radius = min(rect.width(), rect.height()) / 2 - margin

        grid_pen = QPen(QColor("#262c3d"), 1)
        painter.setPen(grid_pen)
        painter.drawLine(cx, margin, cx, rect.height() - margin)
        painter.drawLine(margin, cy, rect.width() - margin, cy)
        painter.setBrush(QBrush(QColor("#262c3d")))
        painter.setPen(Qt.NoPen)
        painter.drawEllipse(int(cx - 4), int(cy - 4), 8, 8)

        if self.vx == 0 and self.vy == 0:
            return

        length = math.sqrt(self.vx * self.vx + self.vy * self.vy)
        if length == 0:
            return

        nx = self.vx / length
        ny = -self.vy / length  # invert y for screen space

        ex = cx + nx * (radius - 10)
        ey = cy + ny * (radius - 10)

        arrow_pen = QPen(QColor("#ff2a2a"), 4)
        painter.setPen(arrow_pen)
        painter.drawLine(int(cx), int(cy), int(ex), int(ey))

        angle = math.atan2(ey - cy, ex - cx)
        head_len = 14
        a1 = angle + math.pi * 0.82
        a2 = angle - math.pi * 0.82

        x1 = ex + math.cos(a1) * head_len
        y1 = ey + math.sin(a1) * head_len
        x2 = ex + math.cos(a2) * head_len
        y2 = ey + math.sin(a2) * head_len

        painter.drawLine(int(ex), int(ey), int(x1), int(y1))
        painter.drawLine(int(ex), int(ey), int(x2), int(y2))


class MainFeaturePanel(QFrame):
    def __init__(self):
        super().__init__()
        self.setObjectName("MainFeaturePanel")

        outer = QVBoxLayout(self)
        outer.setContentsMargins(0, 0, 0, 0)
        outer.setSpacing(0)

        self.hero = QFrame()
        self.hero.setObjectName("HeroFrame")
        hero_layout = QVBoxLayout(self.hero)
        hero_layout.setContentsMargins(20, 20, 20, 20)
        hero_layout.setSpacing(16)

        top_row = QHBoxLayout()
        top_row.setSpacing(12)

        left_block = QVBoxLayout()
        left_block.setSpacing(14)

        self.tag = QLabel("LIVE CONTROL")
        self.tag.setObjectName("TagLabel")
        left_block.addWidget(self.tag, alignment=Qt.AlignLeft)

        self.title = QLabel("How Hamilton rebooted himself in 2026")
        self.title.setObjectName("MainTitle")
        self.title.setWordWrap(True)
        left_block.addWidget(self.title)

        self.subtitle = QLabel("Robot UDP Controller • F1 Styled Interface")
        self.subtitle.setObjectName("MainSubtitle")
        left_block.addWidget(self.subtitle)

        top_row.addLayout(left_block, 1)

        status_card = QFrame()
        status_card.setObjectName("InfoCard")
        status_layout = QVBoxLayout(status_card)
        status_layout.setContentsMargins(14, 12, 14, 12)
        status_layout.setSpacing(6)

        self.conn_label = QLabel("LINK")
        self.conn_label.setObjectName("MiniLabel")
        status_layout.addWidget(self.conn_label)

        self.conn_value = QLabel("IDLE")
        self.conn_value.setObjectName("InfoValue")
        status_layout.addWidget(self.conn_value)

        self.conn_target = QLabel(f"{MCU_IP}:{MCU_PORT}")
        self.conn_target.setObjectName("InfoSub")
        status_layout.addWidget(self.conn_target)

        top_row.addWidget(status_card, 0)

        hero_layout.addLayout(top_row)

        bottom = QHBoxLayout()
        bottom.setSpacing(18)

        # left controls
        controls_col = QVBoxLayout()
        controls_col.setSpacing(16)

        controls_title = QLabel("MOVEMENT")
        controls_title.setObjectName("SectionLabel")
        controls_col.addWidget(controls_title)

        self.key_widgets = {
            "w": KeyBox("W", "w"),
            "a": KeyBox("A", "a"),
            "s": KeyBox("S", "s"),
            "d": KeyBox("D", "d"),
        }

        key_grid_wrap = QFrame()
        key_grid_wrap.setObjectName("SubPanel")
        key_grid = QGridLayout(key_grid_wrap)
        key_grid.setContentsMargins(18, 18, 18, 18)
        key_grid.setHorizontalSpacing(10)
        key_grid.setVerticalSpacing(10)
        key_grid.addWidget(self.key_widgets["w"], 0, 1)
        key_grid.addWidget(self.key_widgets["a"], 1, 0)
        key_grid.addWidget(self.key_widgets["s"], 1, 1)
        key_grid.addWidget(self.key_widgets["d"], 1, 2)

        controls_col.addWidget(key_grid_wrap)

        self.vector_canvas = VectorCanvas()
        vector_wrap = QFrame()
        vector_wrap.setObjectName("SubPanel")
        vector_layout = QVBoxLayout(vector_wrap)
        vector_layout.setContentsMargins(14, 14, 14, 14)
        vector_layout.setSpacing(8)

        vector_label = QLabel("VECTOR")
        vector_label.setObjectName("MiniLabel")
        vector_layout.addWidget(vector_label)

        vector_layout.addWidget(self.vector_canvas)

        self.lbl_vx = QLabel("Vx =   0.00")
        self.lbl_vx.setObjectName("VectorValue")
        vector_layout.addWidget(self.lbl_vx)

        self.lbl_vy = QLabel("Vy =   0.00")
        self.lbl_vy.setObjectName("VectorValue")
        vector_layout.addWidget(self.lbl_vy)

        self.lbl_hex = QLabel("hex: 00000000 00000000")
        self.lbl_hex.setObjectName("HexValue")
        vector_layout.addWidget(self.lbl_hex)

        controls_col.addWidget(vector_wrap, 1)

        bottom.addLayout(controls_col, 1)

        # right telemetry column
        telemetry_col = QVBoxLayout()
        telemetry_col.setSpacing(16)

        telemetry_label = QLabel("TELEMETRY")
        telemetry_label.setObjectName("SectionLabel")
        telemetry_col.addWidget(telemetry_label)

        speed_wrap = QFrame()
        speed_wrap.setObjectName("SubPanel")
        speed_layout = QVBoxLayout(speed_wrap)
        speed_layout.setContentsMargins(16, 16, 16, 16)
        speed_layout.setSpacing(10)

        speed_title = QLabel("SPEED")
        speed_title.setObjectName("MiniLabel")
        speed_layout.addWidget(speed_title)

        self.speed_value = QLabel(f"{int(speed):d}")
        self.speed_value.setObjectName("BigNumber")
        speed_layout.addWidget(self.speed_value)

        self.slider = QSlider(Qt.Horizontal)
        self.slider.setRange(0, 500)
        self.slider.setSingleStep(5)
        self.slider.setValue(int(speed))
        self.slider.valueChanged.connect(self.on_speed_changed)
        speed_layout.addWidget(self.slider)

        scale_row = QHBoxLayout()
        scale_min = QLabel("0")
        scale_min.setObjectName("ScaleHint")
        scale_max = QLabel("500")
        scale_max.setObjectName("ScaleHint")
        scale_row.addWidget(scale_min)
        scale_row.addStretch()
        scale_row.addWidget(scale_max)
        speed_layout.addLayout(scale_row)

        telemetry_col.addWidget(speed_wrap)

        log_wrap = QFrame()
        log_wrap.setObjectName("SubPanel")
        log_layout = QVBoxLayout(log_wrap)
        log_layout.setContentsMargins(16, 16, 16, 16)
        log_layout.setSpacing(8)

        log_title = QLabel("LOG")
        log_title.setObjectName("MiniLabel")
        log_layout.addWidget(log_title)

        self.log_box = QPlainTextEdit()
        self.log_box.setReadOnly(True)
        self.log_box.setObjectName("LogBox")
        self.log_box.setMinimumHeight(280)
        log_layout.addWidget(self.log_box, 1)

        telemetry_col.addWidget(log_wrap, 1)

        bottom.addLayout(telemetry_col, 1)

        hero_layout.addLayout(bottom, 1)

        outer.addWidget(self.hero)

    def on_speed_changed(self, value):
        global speed
        with speed_lock:
            speed = float(value)
        self.speed_value.setText(str(value))
        self.vector_canvas.set_vector(self.vector_canvas.vx, self.vector_canvas.vy, max(speed, 1))

    def set_key_active(self, key_id, active):
        widget = self.key_widgets.get(key_id)
        if widget:
            widget.set_active(active)

    def update_status(self, vx, vy, payload):
        self.lbl_vx.setText(f"Vx = {vx:7.2f}")
        self.lbl_vy.setText(f"Vy = {vy:7.2f}")

        h = payload.hex()
        if len(h) >= 16:
            self.lbl_hex.setText(f"hex: {h[:8]} {h[8:16]}")
        else:
            self.lbl_hex.setText(f"hex: {h}")

        self.vector_canvas.set_vector(vx, vy, max(speed, 1))

        moving = (vx != 0.0 or vy != 0.0)
        self.conn_value.setText("ACTIVE" if moving else "IDLE")
        self.conn_value.setStyleSheet(
            "color: #ff2a2a; font-size: 26px; font-weight: 800;"
            if moving else
            "color: #7f879d; font-size: 26px; font-weight: 800;"
        )

    def append_log(self, msg):
        self.log_box.appendPlainText(msg)
        sb = self.log_box.verticalScrollBar()
        sb.setValue(sb.maximum())


# =========================
# Main Window
# =========================
class F1RobotWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("F1 Robot UDP Controller")
        self.resize(1600, 940)
        self.setMinimumSize(1280, 760)

        central = QWidget()
        self.setCentralWidget(central)

        root = QVBoxLayout(central)
        root.setContentsMargins(0, 0, 0, 0)
        root.setSpacing(0)

        # Header
        header = QFrame()
        header.setObjectName("Header")
        header.setFixedHeight(88)
        header_layout = QHBoxLayout(header)
        header_layout.setContentsMargins(24, 10, 24, 10)
        header_layout.setSpacing(18)

        logo = QLabel("F1")
        logo.setObjectName("Logo")
        header_layout.addWidget(logo)

        for item in ["Schedule", "Results", "News", "Drivers", "Teams", "Fantasy Gaming"]:
            header_layout.addWidget(HeaderButton(item))

        header_layout.addStretch()

        sign_in = QPushButton("Sign In")
        sign_in.setObjectName("SignInButton")
        subscribe = QPushButton("Subscribe")
        subscribe.setObjectName("SubscribeButton")
        header_layout.addWidget(sign_in)
        header_layout.addWidget(subscribe)

        root.addWidget(header)

        # Sub bar
        subbar = QFrame()
        subbar.setObjectName("SubBar")
        subbar.setFixedHeight(58)
        subbar_layout = QHBoxLayout(subbar)
        subbar_layout.setContentsMargins(22, 8, 22, 8)
        subbar_layout.setSpacing(14)

        rlabel = QLabel("R03  |  CONTROL SESSION")
        rlabel.setObjectName("SubInfo")
        venue = QLabel("● Robot Controller")
        venue.setObjectName("CountryInfo")
        subbar_layout.addWidget(rlabel)
        subbar_layout.addWidget(venue)
        subbar_layout.addStretch()

        self.time_label = QLabel("MY TIME  --:--:--")
        self.time_label.setObjectName("SubInfo")
        track = QLabel(f"TRACK TARGET  {MCU_IP}:{MCU_PORT}")
        track.setObjectName("SubInfo")
        subbar_layout.addWidget(self.time_label)
        subbar_layout.addSpacing(18)
        subbar_layout.addWidget(track)

        root.addWidget(subbar)

        # Body
        body = QFrame()
        body.setObjectName("Body")
        body_layout = QHBoxLayout(body)
        body_layout.setContentsMargins(22, 22, 22, 22)
        body_layout.setSpacing(18)

        self.main_panel = MainFeaturePanel()
        self.main_panel.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)

        right_panel = QFrame()
        right_panel.setObjectName("RightPanel")
        right_layout = QVBoxLayout(right_panel)
        right_layout.setContentsMargins(0, 0, 0, 0)
        right_layout.setSpacing(12)

        side_items = [
            ("Fisichella reacts to Antonelli's F1 breakthrough", None),
            ("Why Lawson's China performance showed how far he's come", "UNLOCKED"),
            ("Why has Wheatley left Audi – and is he off to Aston Martin?", None),
            ("Check out Racing Bulls' special livery for Japan", None),
            ("Inside the 2026 F1 title shootout", None),
        ]
        for title, tag in side_items:
            right_layout.addWidget(NewsCard(title, tag))
        right_layout.addStretch()

        body_layout.addWidget(self.main_panel, 2)
        body_layout.addWidget(right_panel, 1)

        root.addWidget(body, 1)

        footer = QLabel(f"UDP  •  little-endian float32 × 2  •  {int(1 / SEND_INTERVAL)} Hz")
        footer.setObjectName("Footer")
        footer.setAlignment(Qt.AlignCenter)
        root.addWidget(footer)

        self.apply_styles()

        bridge.rx_log.connect(self.main_panel.append_log)
        bridge.status.connect(self.main_panel.update_status)

        self.clock_timer = QTimer(self)
        self.clock_timer.timeout.connect(self.update_clock)
        self.clock_timer.start(200)

        self.rx_thread = threading.Thread(target=receive_loop, daemon=True)
        self.tx_thread = threading.Thread(target=send_loop, daemon=True)
        self.rx_thread.start()
        self.tx_thread.start()

        self.update_clock()

    def apply_styles(self):
        self.setStyleSheet("""
            QMainWindow, QWidget {
                background-color: #070b17;
                color: white;
                font-family: Arial, Helvetica, sans-serif;
            }

            #Header {
                background-color: #060b19;
                border-bottom: 1px solid #1a2030;
            }

            #Logo {
                font-size: 34px;
                font-weight: 900;
                color: #ff2a2a;
                padding-right: 22px;
            }

            #HeaderButton {
                background: transparent;
                border: none;
                color: white;
                font-size: 16px;
                padding: 8px 10px;
                text-align: left;
            }

            #HeaderButton:hover {
                color: #ff4d4d;
            }

            #SignInButton {
                background-color: #0d111b;
                color: white;
                border: 1px solid #252b3b;
                border-radius: 18px;
                padding: 8px 16px;
                font-size: 14px;
            }

            #SubscribeButton {
                background-color: #ff2a2a;
                color: white;
                border: none;
                border-radius: 18px;
                padding: 8px 16px;
                font-size: 14px;
                font-weight: 700;
            }

            #SubBar {
                background-color: #000000;
                border-bottom: 1px solid #161b29;
            }

            #SubInfo {
                color: #d1d6e5;
                font-size: 14px;
                font-weight: 700;
            }

            #CountryInfo {
                color: white;
                font-size: 22px;
                font-weight: 800;
            }

            #Body {
                background-color: #070b17;
            }

            #MainFeaturePanel {
                background: transparent;
                border: none;
            }

            #HeroFrame {
                background-color: #090d1c;
                border: 1px solid #151c2d;
                border-radius: 22px;
            }

            #InfoCard {
                background-color: #101626;
                border: 1px solid #222b40;
                border-radius: 16px;
                min-width: 220px;
            }

            #MiniLabel {
                color: #7f879d;
                font-size: 12px;
                font-weight: 700;
                letter-spacing: 1px;
            }

            #InfoValue {
                color: #7f879d;
                font-size: 26px;
                font-weight: 800;
            }

            #InfoSub {
                color: #9aa3b8;
                font-size: 13px;
            }

            #TagLabel {
                background-color: #ff2a2a;
                color: white;
                font-size: 12px;
                font-weight: 800;
                padding: 4px 8px;
                border-radius: 4px;
                max-width: 100px;
            }

            #MainTitle {
                color: white;
                font-size: 36px;
                font-weight: 900;
            }

            #MainSubtitle {
                color: #a3abc0;
                font-size: 15px;
                font-weight: 500;
            }

            #SectionLabel {
                color: white;
                font-size: 14px;
                font-weight: 800;
                letter-spacing: 1px;
            }

            #SubPanel {
                background-color: #0f1422;
                border: 1px solid #1f2738;
                border-radius: 18px;
            }

            #VectorValue {
                color: #ffffff;
                font-size: 18px;
                font-weight: 800;
            }

            #HexValue {
                color: #8f98ae;
                font-size: 13px;
            }

            #BigNumber {
                color: #ff2a2a;
                font-size: 34px;
                font-weight: 900;
            }

            QSlider::groove:horizontal {
                border: 0px;
                height: 8px;
                background: #1f2535;
                border-radius: 4px;
            }

            QSlider::handle:horizontal {
                background: #ff2a2a;
                border: 0px;
                width: 18px;
                margin: -5px 0;
                border-radius: 9px;
            }

            #ScaleHint {
                color: #6f778d;
                font-size: 12px;
            }

            #LogBox {
                background-color: #0a0f1b;
                color: #c4cad9;
                border: 1px solid #1b2233;
                border-radius: 12px;
                padding: 10px;
                font-family: Consolas, monospace;
                font-size: 12px;
            }

            #RightPanel {
                background: transparent;
            }

            #NewsCard {
                background-color: #030509;
                border: 1px solid #141b2a;
                border-radius: 16px;
            }

            #NewsCard:hover {
                border: 1px solid #293249;
            }

            #Thumb {
                background-color: #1a1a1a;
                border-radius: 12px;
            }

            #SideTitle {
                color: white;
                font-size: 15px;
                font-weight: 700;
            }

            #Footer {
                color: #7f879d;
                background-color: #060b19;
                border-top: 1px solid #161d2d;
                padding: 10px;
                font-size: 12px;
                font-weight: 600;
            }
        """)

    def update_clock(self):
        self.time_label.setText(time.strftime("MY TIME  %H:%M:%S"))

    def keyPressEvent(self, event):
        text = event.text().lower()
        if text in KEY_VECTORS:
            with pressed_lock:
                pressed_keys.add(text)
            self.main_panel.set_key_active(text, True)
        super().keyPressEvent(event)

    def keyReleaseEvent(self, event):
        text = event.text().lower()
        if text in KEY_VECTORS:
            with pressed_lock:
                pressed_keys.discard(text)
            self.main_panel.set_key_active(text, False)
        super().keyReleaseEvent(event)

    def closeEvent(self, event):
        try:
            sock.close()
        except Exception:
            pass
        event.accept()


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = F1RobotWindow()
    window.show()
    window.activateWindow()
    window.raise_()
    sys.exit(app.exec())