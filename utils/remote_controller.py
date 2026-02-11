import os
from pynput import keyboard
import socket
import time

# =======================
# 必改：填你机器人（WiFi 板子）串口打印的 IP
ROBOT_IP = "127.0.0.1"
ROBOT_PORT = 5005          # 必须和 MCU 端 Udp.begin(1145) 一致
SEND_HZ = 50               # 每秒发送次数（用于按住键持续发）
# =======================

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)



    
KEYMAP = {
    'w': b'W',
    'a': b'A',
    's': b'S',
    'd': b'D',
}

STOP_CMD = b'X'
BOOST_CMD = b'F'

current_cmd = STOP_CMD
shift_held = False
last_send_t = 0.0


def send(cmd: bytes) -> None:
    """发送 1 字节 UDP 命令到机器人。"""
    try:
        sock.sendto(cmd, (ROBOT_IP, ROBOT_PORT))
    except OSError:
        pass


def on_press(key) -> None:
    global current_cmd, shift_held, last_send_t

    # Esc：发送停止并退出监听
    if key == keyboard.Key.esc:
        current_cmd = STOP_CMD
        send(STOP_CMD)
        # 返回 False 会停止 Listener
        return False

    # Space：停止
    if key == keyboard.Key.space:
        current_cmd = STOP_CMD
        send(STOP_CMD)
        return

    # Shift：加速（按住有效）
    if key in (keyboard.Key.shift, keyboard.Key.shift_l, keyboard.Key.shift_r):
        shift_held = True
        send(BOOST_CMD)
        return

    # 普通键（W/A/S/D）
    if hasattr(key, "char") and key.char:
        c = key.char.lower()
        if c in KEYMAP:
            current_cmd = KEYMAP[c]
            send(current_cmd)
        else:
            print(f"Pressed Unknown char: {key.char}")
    else:
        print(f"Pressed Unknown {key}")


def on_release(key) -> None:
    global current_cmd, shift_held

    # 松开 Shift：取消加速
    if key in (keyboard.Key.shift, keyboard.Key.shift_l, keyboard.Key.shift_r):
        shift_held = False
        return

    # 松开方向键：安全起见直接停
    if hasattr(key, "char") and key.char:
        c = key.char.lower()
        if c in KEYMAP:
            current_cmd = STOP_CMD
            send(STOP_CMD)


def tick_send() -> None:
    """用来在按住方向键时持续发送（不依赖系统键盘 repeat）。"""
    global last_send_t
    now = time.time()
    period = 1.0 / float(SEND_HZ)
    if now - last_send_t >= period:
        last_send_t = now
        # Shift 按住时额外发 F（不覆盖方向）
        send(current_cmd)
        if shift_held:
            send(BOOST_CMD)


# 用 pynput 的 Listener 框架（和你截图完全一致）
with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
    # listener.join() 会阻塞，所以我们用循环 + join(0.01) 方式做“边监听边持续发”
    while listener.is_alive():
        tick_send()
        listener.join(0.01)

# 退出时确保停一下
send(STOP_CMD)
sock.close()





