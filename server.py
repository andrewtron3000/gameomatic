#!/usr/bin/env python3
import random
import socket
import time

frame_bytes = 160 * 128 * 2
port = 2390
payload = bytearray(frame_bytes)
frame_count = 0

# Create a socket.
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind(('', port))
sock.listen(10)

def sleep_print(secs):
   for i in range(secs):
      print(secs - i)
      time.sleep(1)

# Loop waiting for connections (terminate with Ctrl-C)
try:
    print("Listening on {}".format(port))
    while True:
        newSocket, address = sock.accept()
        newSocket.settimeout(2.0)
        newSocket.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, True)
        print("Connected from {}".format(address))
        while True:
            try:
                  for i in range(0, frame_bytes, 4):
                     payload[i] = 0xff
                     payload[i+1] = frame_count & 0xff
                     payload[i+2] = (i >> 8) & 0xff
                     payload[i+3] = i & 0xff
                  newSocket.send(payload)
                  frame_count += 1
                  sleep_print(30)
            except:
                  pass
finally:
    sock.close()
