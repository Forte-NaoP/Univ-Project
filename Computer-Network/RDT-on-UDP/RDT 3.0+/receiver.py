import sys, os, threading, time, socket
import struct

sys.path.append(os.path.dirname(os.path.abspath(os.path.dirname(__file__))))
from logHandler import logHandler

# ----------------------------------------------------------------
# | checkSum (2)| end_flag(1) |  window_Size (1) |   seq_num (4) |
# ----------------------------------------------------------------

PORT_NUMBER = 10090

class Receiver:
    def __init__(self, file_name):
        self.file_name = file_name
        self.seq_num = 0
        self.buffer = None
        self.wsize = 0
        self.base = 0
        self.end_flag = False

    def __enter__(self):
        self.file = open(self.file_name, 'wb')
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.file.close()

    def write(self, payload):
        self.file.write(payload)

    def move_buffer(self):
        i = 0
        idx = (i+self.seq_num) % self.base
        while self.buffer[idx] is not None and i < self.wsize:
            if self.buffer[idx][0] == 0:
                self.end_flag = True
            self.write(self.buffer[idx][1])
            self.buffer[idx] = None
            i += 1
            idx = (idx + 1) % self.base
        self.seq_num = (self.seq_num + i) % self.base

    def ack(self, seq_num):
        return b'\x00'*4 + struct.pack('>I', seq_num)

    def is_end(self):
        return self.end_flag

    def is_in_window(self, seq_num):
        if self.seq_num + self.wsize <= self.base:
            return self.seq_num <= seq_num < self.seq_num + self.wsize
        else:
            return self.seq_num <= seq_num or seq_num < (self.seq_num + self.wsize) % self.base

    @staticmethod
    def verify_checksum(payload):
        if len(payload) % 2 == 1:
            payload += b'\x00'

        checksum = 0
        for i in range(0, len(payload), 2):
            checksum += int.from_bytes(payload[i:i+2], 'big')
            checksum = (checksum >> 16) + (checksum & 0xffff)

        return checksum == 0xffff

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python3 receiver.py <result_file> <log_file>")
        exit(1)
        
    result_file = sys.argv[1]
    log_file = sys.argv[2]

    udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    udp_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    udp_socket.bind(('', PORT_NUMBER))

    logger = logHandler()
    logger.startLogging(log_file)
    with Receiver(result_file) as receiver:
        while (payload := udp_socket.recvfrom(1500)):
            payload, addr = payload

            if (not Receiver.verify_checksum(payload)):
                continue

            end_flag = struct.unpack('>B', payload[2:3])[0]
            window_size = struct.unpack('>B', payload[3:4])[0]
            if receiver.wsize == 0:
                receiver.wsize = window_size
                receiver.base = window_size * 2
                receiver.buffer = [None for _ in range(receiver.base)]
            seq_num = struct.unpack('>I', payload[4:8])[0]

            if receiver.is_in_window(seq_num):
                receiver.buffer[seq_num] = (end_flag, payload[8:])
                if seq_num != receiver.seq_num:
                    logger.writeAck(seq_num, logHandler.WRONG_SEQ_NUM_BUFFER)
                udp_socket.sendto(receiver.ack(seq_num), addr)
                logger.writeAck(seq_num, logHandler.SEND_ACK)

                receiver.move_buffer()
            else:
                continue

            if receiver.is_end():
                break

    logger.writeEnd()