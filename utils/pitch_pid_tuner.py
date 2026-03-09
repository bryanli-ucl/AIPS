#!/usr/bin/env python3
import argparse
import socket
import struct
from typing import Optional


def send_and_wait_ack(sock: socket.socket, payload: bytes, ip: str, port: int, timeout: float) -> None:
    sock.sendto(payload, (ip, port))
    sock.settimeout(timeout)
    try:
        data, addr = sock.recvfrom(1024)
        try:
            txt = data.decode("utf-8", errors="replace").strip()
        except Exception:
            txt = data.hex()
        print(f"[ACK] {addr[0]}:{addr[1]} -> {txt}")
    except socket.timeout:
        print("[WARN] no ACK (timeout)")


def build_text_payload(target: Optional[float], kp: Optional[float], ki: Optional[float], kd: Optional[float]) -> bytes:
    has_target = target is not None
    has_paras = kp is not None and ki is not None and kd is not None

    if has_target and has_paras:
        return f"PITCH {target} {kp} {ki} {kd}".encode("utf-8")
    if has_target and not has_paras:
        return f"TARGET {target}".encode("utf-8")
    if (not has_target) and has_paras:
        return f"PARAS {kp} {ki} {kd}".encode("utf-8")

    raise ValueError("参数组合不合法：要么 target，要么 kp/ki/kd，要么四个都给")


def interactive_loop(sock: socket.socket, ip: str, port: int, timeout: float) -> None:
    print("交互模式：")
    print("  pitch <target> <kp> <ki> <kd>")
    print("  target <target>")
    print("  paras <kp> <ki> <kd>")
    print("  quit")

    while True:
        line = input("> ").strip()
        if not line:
            continue
        if line.lower() in {"q", "quit", "exit"}:
            break

        payload = line.encode("utf-8")
        send_and_wait_ack(sock, payload, ip, port, timeout)


def main() -> None:
    parser = argparse.ArgumentParser(description="Tune master pitch PID over UDP")
    parser.add_argument("--ip", required=True, help="Master board IP, e.g. 192.168.1.210")
    parser.add_argument("--port", type=int, default=9999, help="Master UDP port (default: 9999)")
    parser.add_argument("--target", type=float, help="pitch pid target (rad)")
    parser.add_argument("--kp", type=float, help="pitch pid kp")
    parser.add_argument("--ki", type=float, help="pitch pid ki")
    parser.add_argument("--kd", type=float, help="pitch pid kd")
    parser.add_argument("--binary", action="store_true", help="send 4-float binary packet <ffff>")
    parser.add_argument("--timeout", type=float, default=0.5, help="ack timeout seconds")
    parser.add_argument("--interactive", action="store_true", help="interactive mode")
    args = parser.parse_args()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("", 0))

    try:
        if args.interactive:
            interactive_loop(sock, args.ip, args.port, args.timeout)
            return

        if args.binary:
            if None in (args.target, args.kp, args.ki, args.kd):
                raise ValueError("binary 模式必须给全参数: --target --kp --ki --kd")
            payload = struct.pack("<ffff", args.target, args.kp, args.ki, args.kd)
        else:
            payload = build_text_payload(args.target, args.kp, args.ki, args.kd)

        print(f"[TX] -> {args.ip}:{args.port}  {payload}")
        send_and_wait_ack(sock, payload, args.ip, args.port, args.timeout)
    finally:
        sock.close()


if __name__ == "__main__":
    main()
