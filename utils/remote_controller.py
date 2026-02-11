"""
remote_controller.py (Wi-Fi UDP)

Keyboard -> UDP -> Robot

Controls:
- W/A/S/D : move
- Space   : stop
- Shift   : speed up (send 'F' while held)
- Esc     : stop and exit

Protocol (1 byte per packet):
'W','A','S','D','X','F'
"""

from pynput import keyboard
import socket
import time
import sys

# =======================
# HARD REQUIREMENT: set these
ROBOT_IP = "192.168.1.50"   # <-- change to your robot's IP printed on Serial
ROBOT_PORT = 5005           # must match MCU UDP listening port
SEND_HZ = 50                # 50 Hz => 20 ms period
# =======================

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


def send(sock: socket.socket, cmd: bytes) -> None:
    """Send exactly 1-byte command."""
    try:
        sock.sendto(cmd, (ROBOT_IP, ROBOT_PORT))
    except OSError:
        # Network hiccups shouldn't crash controller; keep running.
        pass


def on_press(key):
    global current_cmd, shift_held

    # ESC: stop & exit listener
    if key == keyboard.Key.esc:
        current_cmd = STOP_CMD
        return False

    # Space: stop (momentary)
    if key == keyboard.Key.space:
        current_cmd = STOP_CMD
        return

    # Shift: boost while held
    if key in (keyboard.Key.shift, keyboard.Key.shift_l, keyboard.Key.shift_r):
        shift_held = True
        return

    # Normal char keys
    if hasattr(key, "char") and key.char:
        c = key.char.lower()
        if c in KEYMAP:
            current_cmd = KEYMAP[c]


def on_release(key):
    global current_cmd, shift_held

    # Release shift -> disable boost
    if key in (keyboard.Key.shift, keyboard.Key.shift_l, keyboard.Key.shift_r):
        shift_held = False
        return

    # Releasing movement key -> stop (safe default)
    if hasattr(key, "char") and key.char:
        c = key.char.lower()
        if c in KEYMAP:
            current_cmd = STOP_CMD


def main() -> int:
    # Basic sanity prints (helps debugging)
    print(f"[remote_controller] Sending UDP to {ROBOT_IP}:{ROBOT_PORT} @ {SEND_HZ} Hz")
    print("Controls: WASD move | Space stop | Shift boost | Esc exit")

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setblocking(False)

    listener = keyboard.Listener(on_press=on_press, on_release=on_release)
    listener.start()

    period = 1.0 / float(SEND_HZ)

    try:
        while listener.is_alive():
            # If shift held, override command with BOOST_CMD (or you can combine in MCU side)
            cmd = BOOST_CMD if shift_held else current_cmd
            send(sock, cmd)
            time.sleep(period)
    except KeyboardInterrupt:
        pass
    finally:
        # Always stop robot on exit
        send(sock, STOP_CMD)
        try:
            listener.stop()
        except Exception:
            pass
        sock.close()
        print("\n[remote_controller] Exited. Sent STOP.")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
