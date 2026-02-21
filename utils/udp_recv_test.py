import socket
import threading
import time
import select

MCU_IP = "192.168.1.209"
MCU_PORT = 9999

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("", 0))
sock.settimeout(1.0)


def receive_loop():
    while True:
        ready, _, _ = select.select([sock], [], [], 0.1)
        if ready:
            data, addr = sock.recvfrom(1024)
            print(f"\n[RX] {addr} -> {data.decode(errors='replace').strip()}")
            print("> ", end="", flush=True)  # 重新打印输入提示符


def send(message: str):
    sock.sendto(message.encode(), (MCU_IP, MCU_PORT))
    print(f"[TX] -> {message.strip()}")


# 启动接收线程
rx_thread = threading.Thread(target=receive_loop, daemon=True)
rx_thread.start()

print(f"UDP 通信已启动")
print(f"  目标: {MCU_IP}:{MCU_PORT}")
print(f"  监听: 0.0.0.0:{sock.getsockname()[1]}")
print("输入消息发送，Ctrl+C 退出\n")

try:
    while True:
        msg = input("> ")
        if msg:
            send(msg)
except KeyboardInterrupt:
    print("\n退出")
finally:
    sock.close()