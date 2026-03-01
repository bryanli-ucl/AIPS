import socket
import threading
import time
import math
import struct
import tkinter as tk
from tkinter import font as tkfont

MCU_IP = "172.20.10.5"
MCU_PORT = 9999
SEND_INTERVAL = 0.1  # 20Hz

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


def receive_loop(log_callback):
    while True:
        try:
            from select import select
            ready, _, _ = select([sock], [], [], 0.1)
            if ready:
                data, addr = sock.recvfrom(1024)
                try:
                    txt = data.decode()
                    if txt != "OK":
                        log_callback(f"[RX] {addr[0]} → {txt}")
                except:
                    log_callback(f"[RX] {addr[0]} → {data.hex()}")
        except:
            pass


def send_loop(status_callback):
    while True:
        vx, vy = compute_vector()
        payload = struct.pack('<ff', vx, vy)
        try:
            sock.sendto(payload, (MCU_IP, MCU_PORT))
        except:
            pass
        status_callback(vx, vy, payload)
        time.sleep(SEND_INTERVAL)


# ─── GUI ──────────────────────────────────────────────────────────────────────

class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Robot UDP Controller")
        self.resizable(False, False)
        self.configure(bg="#0d0d0f")

        self._build_ui()
        self._bind_keys()

        # 启动后台线程
        threading.Thread(target=receive_loop, args=(self._log,), daemon=True).start()
        threading.Thread(target=send_loop,    args=(self._update_status,), daemon=True).start()

    # ── 构建 UI ──────────────────────────────────────────────────────────────

    def _build_ui(self):
        PAD = 16
        BG  = "#0d0d0f"
        PANEL = "#13131a"
        ACC   = "#00e5ff"
        DIM   = "#1e1e2e"

        # ── 顶部标题 ──
        hdr = tk.Frame(self, bg=BG)
        hdr.pack(fill="x", padx=PAD, pady=(PAD, 0))
        tk.Label(hdr, text="ROBOT CTRL", bg=BG, fg=ACC,
                 font=("Courier", 22, "bold")).pack(side="left")
        self.conn_dot = tk.Label(hdr, text="●", bg=BG, fg="#333",
                                 font=("Courier", 14))
        self.conn_dot.pack(side="right", pady=4)
        tk.Label(hdr, text=f"{MCU_IP}:{MCU_PORT}", bg=BG, fg="#555",
                 font=("Courier", 10)).pack(side="right", padx=8)

        sep = tk.Frame(self, bg=ACC, height=1)
        sep.pack(fill="x", padx=PAD, pady=(6, PAD))

        # ── 主体左右 ──
        body = tk.Frame(self, bg=BG)
        body.pack(padx=PAD, pady=0)

        # 左：WASD 图 + 向量显示
        left = tk.Frame(body, bg=BG)
        left.pack(side="left", padx=(0, PAD))

        self._build_wasd(left, PANEL, ACC, DIM)

        # 向量显示
        vec_frame = tk.Frame(left, bg=PANEL, bd=0, relief="flat")
        vec_frame.pack(fill="x", pady=(10, 0), ipady=8, ipadx=10)
        tk.Label(vec_frame, text="VECTOR", bg=PANEL, fg="#555",
                 font=("Courier", 8)).pack()
        self.lbl_vx = tk.Label(vec_frame, text="Vx =   0.00", bg=PANEL, fg=ACC,
                                font=("Courier", 13, "bold"))
        self.lbl_vx.pack()
        self.lbl_vy = tk.Label(vec_frame, text="Vy =   0.00", bg=PANEL, fg=ACC,
                                font=("Courier", 13, "bold"))
        self.lbl_vy.pack()
        self.lbl_hex = tk.Label(vec_frame, text="hex: 00000000 00000000",
                                 bg=PANEL, fg="#444", font=("Courier", 8))
        self.lbl_hex.pack(pady=(2,0))

        # 右：速度滑块 + 日志
        right = tk.Frame(body, bg=BG)
        right.pack(side="left", fill="both")

        # 速度控制
        spd_frame = tk.Frame(right, bg=PANEL)
        spd_frame.pack(fill="x", ipady=10, ipadx=12)
        tk.Label(spd_frame, text="SPEED", bg=PANEL, fg="#555",
                 font=("Courier", 8)).pack()
        self.spd_var = tk.DoubleVar(value=speed)
        self.spd_lbl = tk.Label(spd_frame, text=f"{speed:.0f}", bg=PANEL, fg=ACC,
                                  font=("Courier", 20, "bold"))
        self.spd_lbl.pack()
        slider = tk.Scale(spd_frame, from_=0, to=500, resolution=5,
                          orient="horizontal", variable=self.spd_var,
                          command=self._on_speed_change,
                          bg=PANEL, fg=ACC, troughcolor=DIM,
                          highlightthickness=0, sliderrelief="flat",
                          width=12, length=220, showvalue=False,
                          activebackground=ACC)
        slider.pack(padx=8, pady=(0,4))
        tk.Label(spd_frame, text="0                        500",
                 bg=PANEL, fg="#333", font=("Courier", 7)).pack()

        # 日志
        tk.Label(right, text="LOG", bg=BG, fg="#333",
                 font=("Courier", 8)).pack(anchor="w", pady=(10,2))
        log_frame = tk.Frame(right, bg=PANEL, bd=0)
        log_frame.pack(fill="both", expand=True)
        self.log_box = tk.Text(log_frame, width=34, height=10,
                               bg=PANEL, fg="#888", insertbackground=ACC,
                               font=("Courier", 8), relief="flat", state="disabled",
                               wrap="char")
        self.log_box.pack(padx=6, pady=6)

        # 底部
        tk.Label(self, text=f"UDP  ·  little-endian float32×2  ·  {int(1/SEND_INTERVAL)}Hz",
                 bg=BG, fg="#333", font=("Courier", 7)).pack(pady=(8, PAD))

    def _build_wasd(self, parent, PANEL, ACC, DIM):
        """绘制 WASD 三行按键图"""
        self.key_widgets = {}
        pad = 4
        SZ  = 56

        def make_key(frame, label, row, col, key_id):
            btn = tk.Label(frame, text=label, width=3, height=1,
                           bg=DIM, fg="#555",
                           font=("Courier", 16, "bold"),
                           relief="flat", bd=0)
            btn.grid(row=row, column=col, padx=pad, pady=pad,
                     ipadx=6, ipady=10)
            self.key_widgets[key_id] = btn

        grid = tk.Frame(parent, bg="#0d0d0f")
        grid.pack(pady=(0,0))
        make_key(grid, "W", 0, 1, "w")
        make_key(grid, "A", 1, 0, "a")
        make_key(grid, "S", 1, 1, "s")
        make_key(grid, "D", 1, 2, "d")

        # 方向指示器（canvas）
        self.canvas_size = 120
        cs = self.canvas_size
        self.arrow_canvas = tk.Canvas(parent, width=cs, height=cs,
                                      bg="#13131a", highlightthickness=0)
        self.arrow_canvas.pack(pady=8)
        # 十字
        c = cs // 2
        self.arrow_canvas.create_line(c, 8, c, cs-8, fill="#1e1e2e", width=1)
        self.arrow_canvas.create_line(8, c, cs-8, c, fill="#1e1e2e", width=1)
        self.arrow_canvas.create_oval(c-4, c-4, c+4, c+4, fill="#1e1e2e", outline="")
        # 动态箭头
        self.arrow_line = self.arrow_canvas.create_line(c, c, c, c,
                                                         fill="#00e5ff", width=3,
                                                         arrow="last", arrowshape=(10,12,4))
        self._update_arrow(0.0, 0.0)

    # ── 键盘绑定 ──────────────────────────────────────────────────────────────

    def _bind_keys(self):
        self.bind("<KeyPress>",   self._on_key_press)
        self.bind("<KeyRelease>", self._on_key_release)
        self.focus_set()

    def _on_key_press(self, event):
        k = event.keysym.lower()
        if k in KEY_VECTORS:
            with pressed_lock:
                pressed_keys.add(k)
            self._highlight_key(k, True)

    def _on_key_release(self, event):
        k = event.keysym.lower()
        if k in KEY_VECTORS:
            with pressed_lock:
                pressed_keys.discard(k)
            self._highlight_key(k, False)

    def _highlight_key(self, k, active):
        w = self.key_widgets.get(k)
        if w:
            w.config(bg="#00e5ff" if active else "#1e1e2e",
                     fg="#0d0d0f"  if active else "#555")

    # ── 回调 ─────────────────────────────────────────────────────────────────

    def _on_speed_change(self, val):
        global speed
        v = float(val)
        with speed_lock:
            speed = v
        self.spd_lbl.config(text=f"{v:.0f}")

    def _update_status(self, vx, vy, payload):
        # 从后台线程安全调度到主线程
        self.after(0, self._do_update_status, vx, vy, payload)

    def _do_update_status(self, vx, vy, payload):
        self.lbl_vx.config(text=f"Vx = {vx:7.2f}")
        self.lbl_vy.config(text=f"Vy = {vy:7.2f}")
        h = payload.hex()
        self.lbl_hex.config(text=f"hex: {h[:8]} {h[8:]}")
        self._update_arrow(vx, vy)
        # 连接指示
        moving = vx != 0 or vy != 0
        self.conn_dot.config(fg="#00e5ff" if moving else "#333")

    def _update_arrow(self, vx, vy):
        cs  = self.canvas_size
        c   = cs // 2
        r   = (cs // 2) - 14
        with speed_lock:
            s = speed if speed > 0 else 1
        if vx == 0 and vy == 0:
            self.arrow_canvas.coords(self.arrow_line, c, c, c, c)
            return
        length = math.sqrt(vx*vx + vy*vy)
        nx = vx / length
        ny = -vy / length  # y 轴翻转（屏幕坐标）
        ex = c + nx * r
        ey = c + ny * r
        self.arrow_canvas.coords(self.arrow_line, c, c, ex, ey)

    def _log(self, msg):
        self.after(0, self._do_log, msg)

    def _do_log(self, msg):
        self.log_box.config(state="normal")
        self.log_box.insert("end", msg + "\n")
        self.log_box.see("end")
        self.log_box.config(state="disabled")

    def on_close(self):
        sock.close()
        self.destroy()


if __name__ == "__main__":
    app = App()
    app.protocol("WM_DELETE_WINDOW", app.on_close)
    app.mainloop()
    