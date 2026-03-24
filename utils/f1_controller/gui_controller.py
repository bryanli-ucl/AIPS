import math
import socket
import struct
import sys
import threading
import time
from dataclasses import dataclass
from pathlib import Path
from select import select

from PySide6.QtCore import QObject, QPointF, QRect, Qt, QTimer, Signal
from PySide6.QtGui import QColor, QLinearGradient, QPainter, QPainterPath, QPen, QPixmap
from PySide6.QtWidgets import (
    QApplication,
    QFrame,
    QGridLayout,
    QHBoxLayout,
    QLabel,
    QMainWindow,
    QPushButton,
    QPlainTextEdit,
    QScrollArea,
    QSizePolicy,
    QSlider,
    QVBoxLayout,
    QWidget,
)


MCU_IP = "172.20.10.3"
MCU_PORT = 9999
SEND_INTERVAL = 0.1
MAX_SPEED = 500.0
WINDOW_TITLE = "AIPS Race Control"
WINDOW_WIDTH = 1640
WINDOW_HEIGHT = 960
HERO_HEIGHT = 680
ASSET_IMAGE = Path(__file__).with_name("image.png")
ASSET_CAR_IMAGE = Path(__file__).with_name("f1_car.png")

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("", 0))
sock.settimeout(1.0)

speed = 100.0
pressed_keys = set()
pressed_lock = threading.Lock()
speed_lock = threading.Lock()

KEY_VECTORS = {
    "w": (0.0, 1.0),
    "s": (0.0, -1.0),
    "a": (-1.0, 0.0),
    "d": (1.0, 0.0),
}

ROBOT_PACKET = struct.Struct("<H91bb20s")


@dataclass(slots=True)
class RobotTelemetry:
    item_type: int
    dists: list[int]
    degrees: int
    name: str


def parse_robot_telemetry(data: bytes) -> RobotTelemetry | None:
    if len(data) != ROBOT_PACKET.size:
        return None

    unpacked = ROBOT_PACKET.unpack(data)
    name_bytes = unpacked[93]
    name_str = name_bytes.decode('utf-8', errors='ignore').rstrip('\x00')
    return RobotTelemetry(
        item_type=unpacked[0],
        dists=list(unpacked[1:92]),
        degrees=unpacked[92],
        name=name_str,
    )


def compute_vector():
    with pressed_lock:
        keys = set(pressed_keys)
    if not keys:
        return 0.0, 0.0

    dx = sum(KEY_VECTORS[key][0] for key in keys)
    dy = sum(KEY_VECTORS[key][1] for key in keys)
    length = math.hypot(dx, dy)
    if length == 0:
        return 0.0, 0.0

    with speed_lock:
        current_speed = speed
    return dx / length * current_speed, dy / length * current_speed


class SignalBus(QObject):
    status_changed = Signal(float, float, bytes)
    telemetry_received = Signal(object)
    log_received = Signal(str)


class ControlState:
    def __init__(self):
        self.running = True
        self.paused = False
        self.pause_lock = threading.Lock()


def receive_loop(signals: SignalBus, state: ControlState):
    while state.running:
        try:
            ready, _, _ = select([sock], [], [], 0.1)
            if not ready:
                continue

            data, addr = sock.recvfrom(1024)
            telemetry = parse_robot_telemetry(data)
            if telemetry is not None:
                signals.telemetry_received.emit(telemetry)
                continue
            try:
                text = data.decode(errors="strict")
                if text != "OK":
                    signals.log_received.emit(f"[RX] {addr[0]} -> {text}")
            except UnicodeDecodeError:
                signals.log_received.emit(f"[RX] {addr[0]} -> {data.hex()}")
        except OSError:
            break
        except Exception:
            continue


def send_loop(signals: SignalBus, state: ControlState):
    while state.running:
        with state.pause_lock:
            paused = state.paused

        if paused:
            vx, vy = 0.0, 0.0
        else:
            vx, vy = compute_vector()

        payload = struct.pack("<ff", vx, vy)
        try:
            sock.sendto(payload, (MCU_IP, MCU_PORT))
        except OSError:
            break
        except Exception:
            pass

        signals.status_changed.emit(vx, vy, payload)
        time.sleep(SEND_INTERVAL)


class AccentButton(QPushButton):
    def __init__(self, text: str, accent: bool = False):
        super().__init__(text)
        self.setCursor(Qt.PointingHandCursor)
        self.setProperty("accent", accent)
        self.setObjectName("AccentButton")


class HeroCard(QFrame):
    def __init__(self):
        super().__init__()
        self.setObjectName("HeroCard")
        self.setMinimumHeight(HERO_HEIGHT)
        self.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Preferred)
        self._pixmap = QPixmap(str(ASSET_IMAGE)) if ASSET_IMAGE.exists() else QPixmap()

        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(0)

        self.image_label = QLabel()
        self.image_label.setMinimumHeight(HERO_HEIGHT)
        self.image_label.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Preferred)
        self.image_label.setAlignment(Qt.AlignCenter)
        self.image_label.setObjectName("HeroImage")
        layout.addWidget(self.image_label)

        self.overlay = QWidget(self.image_label)
        self.overlay.setAttribute(Qt.WA_TransparentForMouseEvents)
        self.overlay.setObjectName("HeroOverlay")

        overlay_layout = QVBoxLayout(self.overlay)
        overlay_layout.setContentsMargins(24, 24, 24, 44)
        overlay_layout.setSpacing(8)
        overlay_layout.addStretch()

        self.tag_label = QLabel("LIVE CONTROL")
        self.tag_label.setObjectName("TagLabel")
        overlay_layout.addWidget(self.tag_label, alignment=Qt.AlignLeft)

        self.title_label = QLabel("Race-grade UDP controller for AIPS")
        self.title_label.setWordWrap(True)
        self.title_label.setAlignment(Qt.AlignLeft | Qt.AlignTop)
        self.title_label.setMinimumHeight(96)
        self.title_label.setObjectName("HeroTitle")
        overlay_layout.addWidget(self.title_label, alignment=Qt.AlignLeft)

        self.subtitle_label = QLabel(
            "F1-style control desk with live vector telemetry, keyboard steering and UDP streaming."
        )
        self.subtitle_label.setWordWrap(True)
        self.subtitle_label.setAlignment(Qt.AlignLeft | Qt.AlignTop)
        self.subtitle_label.setMinimumHeight(56)
        self.subtitle_label.setObjectName("HeroSubtitle")
        overlay_layout.addWidget(self.subtitle_label, alignment=Qt.AlignLeft)

        self.refresh_image()

    def resizeEvent(self, event):
        super().resizeEvent(event)
        self.overlay.setGeometry(self.image_label.rect())
        self.refresh_image()

    def refresh_image(self):
        if self._pixmap.isNull():
            self.image_label.setText("AIPS")
            return
        scaled = self._pixmap.scaled(
            self.image_label.size(),
            Qt.KeepAspectRatioByExpanding,
            Qt.SmoothTransformation,
        )
        self.image_label.setPixmap(scaled)


class TelemetryWidget(QWidget):
    def __init__(self):
        super().__init__()
        self.setMinimumHeight(320)
        self.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Preferred)
        self.vx = 0.0
        self.vy = 0.0

    def set_vector(self, vx: float, vy: float):
        self.vx = vx
        self.vy = vy
        self.update()

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)
        rect = self.rect().adjusted(28, 24, -28, -24)

        panel_path = QPainterPath()
        panel_path.addRoundedRect(rect, 18, 18)
        painter.fillPath(panel_path, QColor("#121522"))

        gradient = QLinearGradient(rect.topLeft(), rect.bottomRight())
        gradient.setColorAt(0.0, QColor(255, 255, 255, 16))
        gradient.setColorAt(1.0, QColor(255, 255, 255, 0))
        painter.fillPath(panel_path, gradient)

        center = rect.center()
        radius = min(rect.width(), rect.height()) / 2 - 44

        grid_pen = QPen(QColor("#2A3145"), 1)
        painter.setPen(grid_pen)
        painter.drawEllipse(center, radius, radius)
        painter.drawEllipse(center, radius * 0.55, radius * 0.55)
        painter.drawLine(QPointF(center.x() - radius, center.y()), QPointF(center.x() + radius, center.y()))
        painter.drawLine(QPointF(center.x(), center.y() - radius), QPointF(center.x(), center.y() + radius))

        marker_pen = QPen(QColor("#FF2D2D"), 2)
        painter.setPen(marker_pen)
        painter.drawArc(
            QRect(int(center.x() - radius), int(center.y() - radius), int(radius * 2), int(radius * 2)),
            20 * 16,
            40 * 16,
        )
        painter.drawArc(
            QRect(int(center.x() - radius), int(center.y() - radius), int(radius * 2), int(radius * 2)),
            200 * 16,
            40 * 16,
        )

        painter.setPen(Qt.NoPen)
        painter.setBrush(QColor("#FF2D2D"))
        painter.drawEllipse(center, 5, 5)

        length = math.hypot(self.vx, self.vy)
        if length > 0.0:
            normalized_x = self.vx / max(MAX_SPEED, 1.0)
            normalized_y = self.vy / max(MAX_SPEED, 1.0)
            end_x = center.x() + normalized_x * radius
            end_y = center.y() - normalized_y * radius
            arrow_pen = QPen(QColor("#F6F7FB"), 4, Qt.SolidLine, Qt.RoundCap, Qt.RoundJoin)
            painter.setPen(arrow_pen)
            painter.drawLine(center, QPointF(end_x, end_y))

            angle = math.atan2(center.y() - end_y, end_x - center.x())
            arrow_size = 14
            left = QPointF(
                end_x - arrow_size * math.cos(angle - math.pi / 6),
                end_y + arrow_size * math.sin(angle - math.pi / 6),
            )
            right = QPointF(
                end_x - arrow_size * math.cos(angle + math.pi / 6),
                end_y + arrow_size * math.sin(angle + math.pi / 6),
            )
            painter.drawLine(QPointF(end_x, end_y), left)
            painter.drawLine(QPointF(end_x, end_y), right)


class IRSensorStrip(QWidget):
    def __init__(self):
        super().__init__()
        self.setMinimumHeight(108)
        self.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Preferred)
        self.values = [0.0] * 9

    def set_values(self, values):
        self.values = list(values[:9]) + [0.0] * max(0, 9 - len(values))
        self.update()

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)
        rect = self.rect().adjusted(6, 8, -6, -8)

        panel = QPainterPath()
        panel.addRoundedRect(rect, 18, 18)
        painter.fillPath(panel, QColor("#101420"))

        count = 9
        gap = 12
        lamp_width = (rect.width() - gap * (count - 1)) / count
        lamp_height = rect.height() - 24
        y = rect.y() + (rect.height() - lamp_height) / 2

        for i in range(count):
            x = rect.x() + i * (lamp_width + gap)
            value = max(0.0, min(1.0, self.values[i]))
            shade = int(235 - value * 200)
            lamp_color = QColor(shade, shade, shade)
            border_color = QColor(70, 76, 94)
            lamp_rect = QRect(int(x), int(y), int(lamp_width), int(lamp_height))
            lamp_path = QPainterPath()
            lamp_path.addRoundedRect(lamp_rect, 10, 10)
            painter.fillPath(lamp_path, lamp_color)
            painter.setPen(QPen(border_color, 1))
            painter.drawRoundedRect(lamp_rect, 10, 10)


class RadarWidget(QWidget):
    def __init__(self):
        super().__init__()
        self.setMinimumHeight(260)
        self.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Preferred)
        self.vx = 0.0
        self.vy = 0.0
        self.item_type = 0
        self.dists = [0] * 91
        self.degrees = 0
        self.item_name = ""

    def set_vector(self, vx: float, vy: float):
        self.vx = vx
        self.vy = vy
        self.update()

    def set_scan(self, item_type: int, dists: list[int], degrees: int, name: str = ""):
        self.item_type = item_type
        self.dists = list(dists[:91]) + [0] * max(0, 91 - len(dists))
        self.degrees = max(0, min(90, degrees))
        self.item_name = name
        self.update()

    def _distance_to_radius(self, distance: int, radius: float) -> float:
        if distance <= 0:
            return radius * 0.05
        scaled = math.log2(max(distance, 1)) * 6.0 - 6.0
        return max(radius * 0.08, min(radius, scaled / 55.0 * radius))

    def _fan_point(self, center: QPointF, radius: float, degrees: float) -> QPointF:
        radians = math.radians(degrees)
        return QPointF(
            center.x() + math.sin(radians) * radius,
            center.y() - math.cos(radians) * radius,
        )

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)
        rect = self.rect().adjusted(28, 22, -28, -22)

        panel = QPainterPath()
        panel.addRoundedRect(rect, 18, 18)
        painter.fillPath(panel, QColor("#121522"))

        center = QPointF(rect.center().x(), rect.bottom() - 20)
        radius = min(rect.width() * 0.48, rect.height() - 54)
        fan_rect = QRect(
            int(center.x() - radius),
            int(center.y() - radius),
            int(radius * 2),
            int(radius * 2),
        )

        painter.setPen(QPen(QColor("#25304A"), 1))
        for ring_factor in (1.0, 0.66, 0.33):
            ring_radius = radius * ring_factor
            ring_rect = QRect(
                int(center.x() - ring_radius),
                int(center.y() - ring_radius),
                int(ring_radius * 2),
                int(ring_radius * 2),
            )
            painter.drawArc(ring_rect, 45 * 16, 90 * 16)

        left_edge = self._fan_point(center, radius, -45)
        right_edge = self._fan_point(center, radius, 45)
        painter.drawLine(center, left_edge)
        painter.drawLine(center, right_edge)
        painter.drawLine(left_edge, right_edge)

        painter.setPen(QPen(QColor("#2F3B58"), 1))
        for guide_angle in (-45, -22.5, 0, 22.5, 45):
            painter.drawLine(center, self._fan_point(center, radius, guide_angle))

        sweep_angle = self.degrees - 45
        sweep_end = self._fan_point(center, radius, sweep_angle)
        painter.setPen(QPen(QColor(255, 45, 45, 170), 3))
        painter.drawLine(center, sweep_end)

        point_pen = QPen(QColor("#FF9A3D"), 2)
        painter.setPen(point_pen)
        for degree, distance in enumerate(self.dists):
            if distance <= 0:
                continue
            point_radius = self._distance_to_radius(distance, radius)
            point = self._fan_point(center, point_radius, degree - 45)
            painter.drawPoint(point)

        servo_end = self._fan_point(center, radius, self.degrees - 45)
        painter.setPen(QPen(QColor(255, 255, 255, 80), 1, Qt.DashLine))
        painter.drawLine(center, servo_end)

        normalized_x = self.vx / max(MAX_SPEED, 1.0)
        normalized_y = self.vy / max(MAX_SPEED, 1.0)
        dot = QPointF(
            center.x() + normalized_x * radius * 0.45,
            center.y() - max(0.0, normalized_y) * radius * 0.45,
        )
        painter.setPen(Qt.NoPen)
        painter.setBrush(QColor("#FF2D2D"))
        painter.drawEllipse(dot, 6, 6)

        painter.setPen(QColor("#F6F7FB"))
        item_name = self._item_type_label()
        painter.drawText(QRect(rect.x() + 14, rect.y() + 10, 180, 24), Qt.AlignLeft | Qt.AlignVCenter, item_name)
        painter.setPen(QColor("#8E97AB"))
        painter.drawText(
            QRect(rect.x() + 14, rect.y() + 32, 220, 20),
            Qt.AlignLeft | Qt.AlignVCenter,
            f"mask {self.item_type:09b}   servo {self.degrees:02d} deg",
        )
        painter.drawText(QRect(rect.x() + 10, rect.bottom() - 26, 60, 20), Qt.AlignLeft | Qt.AlignVCenter, "-45")
        painter.drawText(QRect(int(center.x() - 14), rect.y() + 6, 28, 20), Qt.AlignCenter, "0")
        painter.drawText(QRect(rect.right() - 52, rect.bottom() - 26, 42, 20), Qt.AlignRight | Qt.AlignVCenter, "45")

    def _item_type_label(self) -> str:
        if self.item_name:
            return self.item_name
        elif self.item_type == 0:
            return "No Item"
        else:
            return f"Unknown {self.item_type}"



class KeyTile(QFrame):
    def __init__(self, label: str):
        super().__init__()
        self.setObjectName("KeyTile")
        self.setProperty("active", False)

        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)

        self.label = QLabel(label)
        self.label.setAlignment(Qt.AlignCenter)
        self.label.setObjectName("KeyTileLabel")
        layout.addWidget(self.label)

    def set_active(self, active: bool):
        self.setProperty("active", active)
        self.style().unpolish(self)
        self.style().polish(self)
        self.label.setStyleSheet(f"color: {'#071018' if active else '#C6CEDD'};")
        self.update()


class StatCard(QFrame):
    def __init__(self, caption: str, value: str, accent: bool = False):
        super().__init__()
        self.setObjectName("StatCard")
        self.setProperty("accent", accent)
        self.setMinimumHeight(136)
        layout = QVBoxLayout(self)
        layout.setContentsMargins(22, 18, 22, 18)
        layout.setSpacing(10)

        self.caption_label = QLabel(caption)
        self.caption_label.setObjectName("CaptionLabel")
        layout.addWidget(self.caption_label)

        self.value_label = QLabel(value)
        self.value_label.setObjectName("StatValue")
        layout.addWidget(self.value_label)

    def set_value(self, value: str):
        self.value_label.setText(value)


class SectionCard(QFrame):
    def __init__(self, title: str, content: QWidget):
        super().__init__()
        self.setObjectName("SectionCard")
        self.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Preferred)

        layout = QVBoxLayout(self)
        layout.setContentsMargins(18, 14, 18, 14)
        layout.setSpacing(10)

        title_label = QLabel(title)
        title_label.setObjectName("CaptionLabel")
        layout.addWidget(title_label)
        layout.addWidget(content)


class PhotoCard(QFrame):
    def __init__(self, image_path: Path, title: str = "F1 Car"):
        super().__init__()
        self.setObjectName("SectionCard")
        self._image_path = image_path
        self._pixmap = QPixmap(str(image_path)) if image_path.exists() else QPixmap()

        layout = QVBoxLayout(self)
        layout.setContentsMargins(18, 14, 18, 18)
        layout.setSpacing(12)

        title_label = QLabel(title)
        title_label.setObjectName("CaptionLabel")
        layout.addWidget(title_label)

        self.image_label = QLabel()
        self.image_label.setObjectName("CarPhoto")
        self.image_label.setAlignment(Qt.AlignCenter)
        self.image_label.setMinimumHeight(280)
        self.image_label.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Preferred)
        layout.addWidget(self.image_label)

        self._refresh()

    def resizeEvent(self, event):
        super().resizeEvent(event)
        self._refresh()

    def _refresh(self):
        if self._pixmap.isNull():
            self.image_label.setText("Put an F1 car image at utils/f1_car.png")
            return

        scaled = self._pixmap.scaled(
        self.image_label.size(),
        Qt.KeepAspectRatio,
    Qt.SmoothTransformation,
)

        self.image_label.setPixmap(scaled)


class ControlWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle(WINDOW_TITLE)
        self.resize(WINDOW_WIDTH, WINDOW_HEIGHT)
        self.setFixedSize(WINDOW_WIDTH, WINDOW_HEIGHT)
        self.setFocusPolicy(Qt.StrongFocus)

        self.signals = SignalBus()
        self.state = ControlState()
        self.key_tiles = {}
        self.last_status_at = 0.0

        self._build_ui()
        self._connect_signals()
        self._start_threads()

        self.setFocus()
        self._refresh_status_badge()
        self._append_log(f"[SYS] Controller ready -> {MCU_IP}:{MCU_PORT}")

    def _build_ui(self):
        root = QWidget()
        self.setCentralWidget(root)

        main_layout = QVBoxLayout(root)
        main_layout.setContentsMargins(0, 0, 0, 0)
        main_layout.setSpacing(0)

        main_layout.addWidget(self._build_top_header())
        main_layout.addWidget(self._build_sub_header())

        body = QWidget()
        body.setFixedWidth(WINDOW_WIDTH - 18)
        body.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Preferred)
        body_layout = QHBoxLayout(body)
        body_layout.setContentsMargins(36, 28, 36, 42)
        body_layout.setSpacing(26)

        left_panel = QWidget()
        left_panel.setFixedWidth(1320)
        left_layout = QVBoxLayout(left_panel)
        left_layout.setContentsMargins(0, 0, 0, 0)
        left_layout.setSpacing(26)

        self.hero_card = HeroCard()
        left_layout.addWidget(self.hero_card)
        left_layout.addWidget(self._build_control_card())
        left_layout.addStretch()

        right_panel = QWidget()
        right_panel.setFixedWidth(220)
        right_layout = QVBoxLayout(right_panel)
        right_layout.setContentsMargins(0, 0, 0, 0)
        right_layout.setSpacing(22)
        status_card = self._build_status_card()
        status_card.setMinimumHeight(420)
        log_card = self._build_log_card()
        log_card.setMinimumHeight(520)
        right_layout.addWidget(status_card)
        right_layout.addWidget(log_card)
        right_layout.addStretch()

        body_layout.addWidget(left_panel)
        body_layout.addWidget(right_panel)

        scroll = QScrollArea()
        scroll.setObjectName("BodyScroll")
        scroll.setWidgetResizable(False)
        scroll.setFrameShape(QFrame.NoFrame)
        scroll.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        scroll.setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)
        scroll.setWidget(body)
        main_layout.addWidget(scroll, 1)
        body.adjustSize()

        self.setStyleSheet(self._stylesheet())

        self.connection_timer = QTimer(self)
        self.connection_timer.timeout.connect(self._refresh_status_badge)
        self.connection_timer.start(250)

    def _build_top_header(self):
        frame = QFrame()
        frame.setObjectName("TopHeader")
        layout = QHBoxLayout(frame)
        layout.setContentsMargins(34, 12, 34, 12)
        layout.setSpacing(22)

        logo = QLabel("F1")
        logo.setObjectName("LogoLabel")
        layout.addWidget(logo)

        for title in ("Schedule", "Results", "Telemetry", "Drivers", "Teams", "Control Desk"):
            button = AccentButton(title)
            layout.addWidget(button)

        layout.addStretch()
        layout.addWidget(AccentButton("Connect"))
        layout.addWidget(AccentButton("Deploy", accent=True))
        return frame

    def _build_sub_header(self):
        frame = QFrame()
        frame.setObjectName("SubHeader")
        layout = QHBoxLayout(frame)
        layout.setContentsMargins(34, 10, 34, 10)
        layout.setSpacing(18)

        round_label = QLabel("R03  |  LIVE SESSION")
        round_label.setObjectName("SubtleLabel")
        layout.addWidget(round_label)

        circuit_label = QLabel("AIPS UDP CONTROL")
        circuit_label.setObjectName("TrackLabel")
        layout.addWidget(circuit_label)

        layout.addStretch()

        self.connection_dot = QLabel("●")
        self.connection_dot.setObjectName("ConnectionDot")
        layout.addWidget(self.connection_dot)

        self.connection_label = QLabel("STANDBY")
        self.connection_label.setObjectName("SubtleLabel")
        layout.addWidget(self.connection_label)

        endpoint = QLabel(f"{MCU_IP}:{MCU_PORT}")
        endpoint.setObjectName("EndpointLabel")
        layout.addWidget(endpoint)
        return frame

    def _build_control_card(self):
        panel = QFrame()
        panel.setObjectName("Panel")
        panel.setMinimumHeight(1360)
        panel.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Preferred)
        layout = QVBoxLayout(panel)
        layout.setContentsMargins(26, 24, 26, 24)
        layout.setSpacing(20)

        title = QLabel("Control Stack")
        title.setObjectName("PanelTitle")
        layout.addWidget(title)

        subtitle = QLabel("Keyboard steering, live vector output and UDP payload monitor.")
        subtitle.setWordWrap(True)
        subtitle.setObjectName("PanelText")
        layout.addWidget(subtitle)

        stats_row = QHBoxLayout()
        stats_row.setSpacing(14)
        self.speed_card = StatCard("Speed", f"{speed:.0f}", accent=True)
        self.vx_card = StatCard("Vx", "0.00")
        self.vy_card = StatCard("Vy", "0.00")
        stats_row.addWidget(self.speed_card)
        stats_row.addWidget(self.vx_card)
        stats_row.addWidget(self.vy_card)
        layout.addLayout(stats_row)

        content_row = QHBoxLayout()
        content_row.setSpacing(28)

        left_stack = QVBoxLayout()
        left_stack.setSpacing(16)

        slider_caption = QLabel("Speed Trim")
        slider_caption.setObjectName("CaptionLabel")
        left_stack.addWidget(slider_caption)

        self.speed_slider = QSlider(Qt.Horizontal)
        self.speed_slider.setRange(0, int(MAX_SPEED))
        self.speed_slider.setSingleStep(5)
        self.speed_slider.setPageStep(25)
        self.speed_slider.setValue(int(speed))
        self.speed_slider.valueChanged.connect(self._on_speed_changed)
        left_stack.addWidget(self.speed_slider)

        scale_row = QHBoxLayout()
        scale_row.addWidget(self._make_small_label("0"))
        scale_row.addStretch()
        scale_row.addWidget(self._make_small_label(str(int(MAX_SPEED))))
        left_stack.addLayout(scale_row)

        self.hex_label = QLabel("Payload  00000000 00000000")
        self.hex_label.setObjectName("HexLabel")
        left_stack.addWidget(self.hex_label)

        note = QLabel("Press W A S D while this window is focused.")
        note.setObjectName("PanelText")
        note.setWordWrap(True)
        left_stack.addWidget(note)

        self.pause_button = QPushButton("Pause Output")
        self.pause_button.setCheckable(True)
        self.pause_button.setObjectName("PauseButton")
        self.pause_button.toggled.connect(self._toggle_pause)
        left_stack.addWidget(self.pause_button)
        left_stack.addStretch()

        right_stack = QVBoxLayout()
        right_stack.setSpacing(18)

        self.telemetry = TelemetryWidget()
        right_stack.addWidget(SectionCard("Vector Plot", self.telemetry))

        self.radar = RadarWidget()
        right_stack.addWidget(SectionCard("Radar Sweep", self.radar))

        content_row.addLayout(left_stack, 5)
        content_row.addLayout(right_stack, 4)
        layout.addLayout(content_row)

        self.car_photo_card = PhotoCard(ASSET_CAR_IMAGE, "F1 Car Photo")
        layout.addWidget(self.car_photo_card)

        ir_panel = QFrame()
        ir_panel.setObjectName("Panel")
        ir_panel.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Preferred)
        ir_layout = QVBoxLayout(ir_panel)
        ir_layout.setContentsMargins(26, 24, 26, 24)
        ir_layout.setSpacing(16)

        ir_title = QLabel("IR Sensor Detection")
        ir_title.setObjectName("PanelTitle")
        ir_layout.addWidget(ir_title)

        self.ir_name_card = QFrame()
        self.ir_name_card.setObjectName("SectionCard")
        self.ir_name_card.setMinimumHeight(100)
        ir_name_layout = QVBoxLayout(self.ir_name_card)
        ir_name_layout.setContentsMargins(20, 16, 20, 16)
        ir_name_layout.setSpacing(8)

        ir_label = QLabel("Operater")
        ir_label.setObjectName("CaptionLabel")
        ir_name_layout.addWidget(ir_label)

        self.ir_object_name = QLabel("No Detection")
        self.ir_object_name.setObjectName("StatValue")
        self.ir_object_name.setAlignment(Qt.AlignCenter)
        font = self.ir_object_name.font()
        font.setPointSize(28)
        font.setBold(True)
        self.ir_object_name.setFont(font)
        ir_name_layout.addWidget(self.ir_object_name)

        ir_layout.addWidget(self.ir_name_card)

        ir_sensor_label = QLabel("Sensor Strip")
        ir_sensor_label.setObjectName("CaptionLabel")
        ir_layout.addWidget(ir_sensor_label)

        self.ir_strip = IRSensorStrip()
        ir_layout.addWidget(self.ir_strip)

        layout.addWidget(ir_panel)

        keys_frame = QFrame()
        keys_frame.setObjectName("KeysFrame")
        keys_frame.setMinimumHeight(360)
        keys_frame.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Preferred)
        keys_layout = QVBoxLayout(keys_frame)
        keys_layout.setContentsMargins(22, 18, 22, 20)
        keys_layout.setSpacing(16)

        keys_title = QLabel("Drive Keys")
        keys_title.setObjectName("CaptionLabel")
        keys_layout.addWidget(keys_title)

        key_row = QHBoxLayout()
        key_row.setSpacing(46)
        key_row.setContentsMargins(0, 18, 0, 24)
        key_row.addStretch()

        key_cluster = QGridLayout()
        key_cluster.setHorizontalSpacing(34)
        key_cluster.setVerticalSpacing(30)
        self._add_key_tile(key_cluster, "W", "w", 0, 1)
        self._add_key_tile(key_cluster, "A", "a", 1, 0)
        self._add_key_tile(key_cluster, "S", "s", 1, 1)
        self._add_key_tile(key_cluster, "D", "d", 1, 2)
        key_row.addLayout(key_cluster)
        key_row.addStretch()

        keys_layout.addLayout(key_row)
        layout.addWidget(keys_frame)

        return panel

    def _build_status_card(self):
        panel = QFrame()
        panel.setObjectName("Panel")
        layout = QVBoxLayout(panel)
        layout.setContentsMargins(20, 20, 20, 20)
        layout.setSpacing(12)

        title = QLabel("Session")
        title.setObjectName("PanelTitle")
        layout.addWidget(title)

        lines = [
            ("Endpoint", f"{MCU_IP}:{MCU_PORT}"),
            ("Protocol", "UDP little-endian float32 x2"),
            ("Send Rate", f"{int(round(1 / SEND_INTERVAL))} Hz"),
            ("Control", "Keyboard vector drive"),
        ]

        for label_text, value_text in lines:
            caption = QLabel(label_text)
            caption.setObjectName("CaptionLabel")
            layout.addWidget(caption)

            value = QLabel(value_text)
            value.setObjectName("PanelText")
            value.setWordWrap(True)
            layout.addWidget(value)

        layout.addStretch()
        return panel

    def _build_log_card(self):
        panel = QFrame()
        panel.setObjectName("Panel")
        layout = QVBoxLayout(panel)
        layout.setContentsMargins(20, 20, 20, 20)
        layout.setSpacing(14)

        title = QLabel("Radio / Log")
        title.setObjectName("PanelTitle")
        layout.addWidget(title)

        info = QLabel("Incoming UDP messages and controller state updates.")
        info.setObjectName("PanelText")
        info.setWordWrap(True)
        layout.addWidget(info)

        self.log_box = QPlainTextEdit()
        self.log_box.setReadOnly(True)
        self.log_box.setMaximumBlockCount(200)
        self.log_box.setObjectName("LogBox")
        self.log_box.setMinimumHeight(360)
        layout.addWidget(self.log_box, 1)
        return panel

    def _add_key_tile(self, layout, title, key_id, row, col):
        tile = KeyTile(title)
        tile.setFixedSize(112, 88)
        layout.addWidget(tile, row, col)
        self.key_tiles[key_id] = tile

    def _make_small_label(self, text: str):
        label = QLabel(text)
        label.setObjectName("ScaleLabel")
        return label

    def _connect_signals(self):
        self.signals.status_changed.connect(self._update_status)
        self.signals.telemetry_received.connect(self._update_robot_telemetry)
        self.signals.log_received.connect(self._append_log)

    def _start_threads(self):
        self.receive_thread = threading.Thread(
            target=receive_loop,
            args=(self.signals, self.state),
            daemon=True,
        )
        self.send_thread = threading.Thread(
            target=send_loop,
            args=(self.signals, self.state),
            daemon=True,
        )
        self.receive_thread.start()
        self.send_thread.start()

    def _handle_key_change(self, qt_key: int, active: bool):
        mapping = {
            Qt.Key_W: "w",
            Qt.Key_A: "a",
            Qt.Key_S: "s",
            Qt.Key_D: "d",
        }
        key_id = mapping.get(qt_key)
        if not key_id:
            return

        with pressed_lock:
            if active:
                pressed_keys.add(key_id)
            else:
                pressed_keys.discard(key_id)

        tile = self.key_tiles.get(key_id)
        if tile:
            tile.set_active(active)

    def keyPressEvent(self, event):
        if not event.isAutoRepeat():
            self._handle_key_change(event.key(), True)
        super().keyPressEvent(event)

    def keyReleaseEvent(self, event):
        if not event.isAutoRepeat():
            self._handle_key_change(event.key(), False)
        super().keyReleaseEvent(event)

    def _on_speed_changed(self, value: int):
        global speed
        with speed_lock:
            speed = float(value)
        self.speed_card.set_value(f"{value:.0f}")

    def _update_status(self, vx: float, vy: float, payload: bytes):
        self.last_status_at = time.time()
        self.vx_card.set_value(f"{vx:0.2f}")
        self.vy_card.set_value(f"{vy:0.2f}")
        hex_payload = payload.hex()
        self.hex_label.setText(f"Payload  {hex_payload[:8]} {hex_payload[8:]}")
        self.telemetry.set_vector(vx, vy)
        self.radar.set_vector(vx, vy)
        self._refresh_status_badge()

    def _update_robot_telemetry(self, telemetry: RobotTelemetry):
        self.last_status_at = time.time()
        self.radar.set_scan(telemetry.item_type, telemetry.dists, telemetry.degrees, telemetry.name)
        self.ir_strip.set_values(self._decode_item_type_bits(telemetry.item_type))
        display_name = telemetry.name.strip() if telemetry.name else "No Detection"
        self.ir_object_name.setText(display_name if display_name else "No Detection")
        if display_name and display_name != "No Detection":
            self.ir_name_card.setProperty("accent", True)
        else:
            self.ir_name_card.setProperty("accent", False)
        self.ir_name_card.style().unpolish(self.ir_name_card)
        self.ir_name_card.style().polish(self.ir_name_card)
        self._refresh_status_badge()

    def _decode_item_type_bits(self, item_type: int):
        return [1.0 if (item_type >> index) & 0x1 else 0.0 for index in range(9)]

    def _toggle_pause(self, checked: bool):
        with self.state.pause_lock:
            self.state.paused = checked
        self.pause_button.setText("Resume Output" if checked else "Pause Output")
        self._append_log("[SYS] Output paused" if checked else "[SYS] Output resumed")

    def _refresh_status_badge(self):
        active = time.time() - self.last_status_at < SEND_INTERVAL * 3
        self.connection_dot.setStyleSheet(f"color: {'#FF2D2D' if active else '#4A5168'};")
        self.connection_label.setText("STREAMING" if active else "STANDBY")

    def _append_log(self, message: str):
        self.log_box.appendPlainText(message)

    def closeEvent(self, event):
        self.state.running = False
        try:
            sock.close()
        except OSError:
            pass
        super().closeEvent(event)

    def _stylesheet(self):
        return """
        QWidget {
            background: #070A12;
            color: #F5F7FB;
            font-family: "Segoe UI", "Microsoft YaHei", sans-serif;
        }

        QMainWindow {
            background: #070A12;
        }

        #TopHeader {
            background-color: #11141E;
            border-bottom: 1px solid #242A3C;
        }

        #SubHeader {
            background-color: #050608;
            border-bottom: 1px solid #1B2130;
        }

        #LogoLabel {
            color: #FF2D2D;
            font-size: 34px;
            font-weight: 900;
            letter-spacing: 1px;
            padding-right: 16px;
        }

        #AccentButton {
            background: transparent;
            border: none;
            color: #F4F6FB;
            font-size: 15px;
            font-weight: 600;
            padding: 8px 10px;
        }

        #AccentButton:hover {
            color: #FF5858;
        }

        #AccentButton[accent="true"] {
            background: #FF2D2D;
            border-radius: 18px;
            padding: 10px 18px;
        }

        #AccentButton[accent="true"]:hover {
            background: #FF4444;
            color: white;
        }

        #SubtleLabel, #EndpointLabel {
            color: #C9D0DE;
            font-size: 13px;
            font-weight: 600;
        }

        #TrackLabel {
            color: white;
            font-size: 22px;
            font-weight: 800;
            letter-spacing: 0.5px;
        }

        #ConnectionDot {
            font-size: 18px;
            font-weight: 900;
            color: #4A5168;
        }

        #HeroCard {
            background: transparent;
            border: none;
        }

        #HeroImage {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #151A28, stop:1 #090C14);
            border-radius: 24px;
            border: 1px solid #202638;
        }

        #HeroOverlay {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(7,10,18,18),
                stop:0.55 rgba(7,10,18,36),
                stop:1 rgba(7,10,18,220));
            border-radius: 24px;
        }

        #TagLabel {
            background: #FF2D2D;
            color: white;
            font-size: 12px;
            font-weight: 900;
            padding: 5px 9px;
            border-radius: 5px;
        }

        #HeroTitle {
            background: transparent;
            color: white;
            font-size: 20px;
            font-weight: 600;
        }

        #HeroSubtitle {
            background: transparent;
            color: #D6DCEA;
            font-size: 11px;
            font-weight: 500;
            max-width: 520px;
        }

        #Panel {
            background: #101420;
            border: 1px solid #1F2638;
            border-radius: 22px;
        }

        #PanelTitle {
            color: white;
            font-size: 24px;
            font-weight: 800;
        }

        #PanelText, #ScaleLabel, #HexLabel {
            color: #9DA7BC;
            font-size: 14px;
            font-weight: 500;
        }

        #CaptionLabel {
            color: #E6EAF3;
            font-size: 13px;
            font-weight: 700;
            text-transform: uppercase;
            letter-spacing: 1px;
        }

        #StatCard {
            background: #0A0D16;
            border: 1px solid #20273A;
            border-radius: 16px;
        }

        #StatCard[accent="true"] {
            border: 1px solid #6C1D23;
            background: #181019;
        }

        #StatValue {
            color: white;
            font-size: 32px;
            font-weight: 800;
        }

        #SectionCard {
            background: #0D111B;
            border: 1px solid #1F2638;
            border-radius: 18px;
        }

        #CarPhoto {
            background: #090C14;
            border: 1px solid #1F2638;
            border-radius: 16px;
            color: #7F8AA3;
            font-size: 14px;
            padding: 8px;
        }

        #KeysFrame {
            background: #0D111B;
            border: 1px solid #1F2638;
            border-radius: 18px;
        }

        #PauseButton {
            background: #151A28;
            color: white;
            border: 1px solid #293249;
            border-radius: 14px;
            padding: 12px 16px;
            font-size: 14px;
            font-weight: 700;
        }

        #PauseButton:checked {
            background: #FF2D2D;
            border: 1px solid #FF6B6B;
        }

        #KeyTile {
            background: #0B0E17;
            border: 1px solid #252B3B;
            border-radius: 18px;
        }

        #KeyTile[active="true"] {
            background: #FF2D2D;
            border: 1px solid #FF6B6B;
        }

        #KeyTileLabel {
            color: #C6CEDD;
            font-size: 32px;
            font-weight: 900;
        }

        #LogBox {
            background: #090B12;
            border: 1px solid #1B2130;
            border-radius: 16px;
            padding: 12px;
            color: #C9D0DE;
            font-family: Consolas, "Courier New", monospace;
            font-size: 12px;
        }

        QSlider::groove:horizontal {
            border: none;
            height: 8px;
            background: #1D2433;
            border-radius: 4px;
        }

        QSlider::sub-page:horizontal {
            background: #FF2D2D;
            border-radius: 4px;
        }

        QSlider::handle:horizontal {
            background: #F7F9FC;
            width: 18px;
            margin: -6px 0;
            border-radius: 9px;
        }
        """


def main():
    app = QApplication(sys.argv)
    window = ControlWindow()
    window.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()
