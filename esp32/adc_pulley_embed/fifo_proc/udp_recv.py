#!/usr/bin/env python3
import socket

LISTEN_IP = "0.0.0.0"   # listen on all host interfaces
LISTEN_PORT = 9000
BUF_SIZE = 2048

def main() -> None:
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((LISTEN_IP, LISTEN_PORT))

    print(f"Listening for UDP on {LISTEN_IP}:{LISTEN_PORT}")

    while True:
        data, addr = sock.recvfrom(BUF_SIZE)
        print(f"\nReceived {len(data)} bytes from {addr[0]}:{addr[1]}")
        print("Hex :", data.hex(" "))
        # try:
        #     print("Text:", data.decode("utf-8"))
        # except UnicodeDecodeError:
        #     print("Text: <not valid UTF-8>")

if __name__ == "__main__":
    main()