import sys, os, threading, time, socket
import struct

# ----------------------------------------------
# | end_flag(1) |  seq_num (1) |  ...payload   |
# ----------------------------------------------
PORT_NUMBER = 10090

class Receiver():
    def __init__(self, file_name):
        self.file_name = file_name

    def __enter__(self):
        self.file = open(self.file_name, 'wb')
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.file.close()

    def write(self, payload):
        self.file.write(payload)


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python3 receiver.py <result_file> <log_file>")
        exit(1)
        
    result_file = sys.argv[1]
    log_file = sys.argv[2]

    udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    udp_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    udp_socket.bind(('', PORT_NUMBER))
    rcv = 0
    with Receiver(result_file) as receiver:
        while (payload := udp_socket.recvfrom(1500)):
            payload, addr = payload
            end_flag = struct.unpack('>B', payload[0:1])[0]
            receiver.write(payload[2:])
            if end_flag == 0:
                break
