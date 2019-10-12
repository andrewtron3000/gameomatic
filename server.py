#!/usr/bin/env python3
import random
import socket
import time

frame_size = 160 * 128 * 2
port = 2390

# Create a socket.
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind(('', port))
sock.listen(10)

# Loop waiting for connections (terminate with Ctrl-C)
try:
    print("Listening on {}".format(port))
    while True:
        newSocket, address = sock.accept()
        newSocket.settimeout(5.0)
        newSocket.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, True)
        print("Connected from {}".format(address))

        # loop serving the new client
        while True:
            try:
                print("Got here A")
                receivedData = newSocket.recv(2)
                print("Got here B ({})".format(receivedData))
                time.sleep(0.1)

                for i in range(10):
                    payload = bytearray(
                        random.getrandbits(8) for _ in range(frame_size // 10))
                    newSocket.send(payload)
                print("Got here C")
            except Exception as e:
                newSocket.close()
                print("Disconnected from {}: {}".format(address, e))
                break
finally:
    sock.close()
