import socket
import threading
import time
import select
import math
import sys
import tty
import termios
import struct

MCU_IP = "192.168.1.210"
MCU_PORT = 9999
SEND_INTERVAL = 0.2  # 发送间隔 20Hz

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("", 0))
sock.settimeout(1.0)

speed = 100.0          # 当前速度模长，可通过命令调节
pressed_keys = set()
pressed_lock = threading.Lock()
speed_lock = threading.Lock()

KEY_VECTORS = {
    "UP":    (0.0,  1.0),
    "DOWN":  (0.0, -1.0),
    "LEFT":  (-1.0, 0.0),
    "RIGHT": (1.0,  0.0),
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


def receive_loop():
    while True:
        ready, _, _ = select.select([sock], [], [], 0.1)
        if ready:
            data, addr = sock.recvfrom(1024)
            if data.decode() != 'OK' :
                print(f"\n[RX] {addr} -> {data.hex()}")


def send_loop():
    while True:
        vx, vy = compute_vector()
        payload = struct.pack('<ff', vx, vy)
        sock.sendto(payload, (MCU_IP, MCU_PORT))
        print(f"\r[TX] vx={vx:7.2f}, vy={vy:7.2f}  [{payload.hex()}]", end="", flush=True)
        time.sleep(SEND_INTERVAL)


def read_key():
    fd = sys.stdin.fileno()
    old = termios.tcgetattr(fd)
    try:
        tty.setraw(fd)
        ch = sys.stdin.read(1)
        if ch == '\x1b':
            ch2 = sys.stdin.read(1)
            if ch2 == '[':
                ch3 = sys.stdin.read(1)
                return {"A": "UP", "B": "DOWN", "C": "RIGHT", "D": "LEFT"}.get(ch3, "")
        return ch
    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old)


def keyboard_loop():
    cmd_buf = ""

    while True:
        key = read_key()

        if key == '\x03':
            raise KeyboardInterrupt

        if key == '\r' or key == '\n':
            cmd = cmd_buf.strip()
            cmd_buf = ""
            if cmd:
                handle_command(cmd)
            continue

        if key == '\x7f':
            if cmd_buf:
                cmd_buf = cmd_buf[:-1]
                print(f"\r> {cmd_buf}  \r> {cmd_buf}", end="", flush=True)
            continue

        if key in KEY_VECTORS:
            with pressed_lock:
                pressed_keys.add(key)
        elif key and key.isprintable():
            cmd_buf += key
            print(f"\r> {cmd_buf}", end="", flush=True)


def handle_command(cmd: str):
    """
    支持的命令：
      speed <value>   —— 设置速度模长，如 speed 200
      speed           —— 查询当前速度
    """
    global speed
    parts = cmd.lower().split()
    if not parts:
        return
    if parts[0] == "speed":
        if len(parts) == 1:
            with speed_lock:
                print(f"\n[INFO] 当前速度模长: {speed}")
        else:
            try:
                val = float(parts[1])
                if val < 0:
                    raise ValueError
                with speed_lock:
                    speed = val
                print(f"\n[INFO] 速度模长已设为: {speed}")
            except ValueError:
                print(f"\n[ERR] 无效的速度值: {parts[1]}")
    else:
        print(f"\n[ERR] 未知命令: {cmd}，支持: speed <value>")


# 启动各后台线程
threading.Thread(target=receive_loop, daemon=True).start()
threading.Thread(target=send_loop, daemon=True).start()

print(f"UDP 控制端已启动")
print(f"  目标: {MCU_IP}:{MCU_PORT}")
print(f"  监听: 0.0.0.0:{sock.getsockname()[1]}")
print(f"  初始速度模长: {speed}，发送频率: {int(1/SEND_INTERVAL)}Hz")
print(f"  数据格式: little-endian float32 x2 (8 bytes)")
print(f"  命令: speed <value> 调节速度，Ctrl+C 退出\n")

try:
    keyboard_loop()
except KeyboardInterrupt:
    print("\n退出")
finally:
    sock.close()