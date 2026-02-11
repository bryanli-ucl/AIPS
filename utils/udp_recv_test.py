import socket

PORT = 1145
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(("0.0.0.0", PORT))
print(f"LISTEN {PORT}")

while True:
    data, addr = s.recvfrom(1024)
    print(addr, data)
