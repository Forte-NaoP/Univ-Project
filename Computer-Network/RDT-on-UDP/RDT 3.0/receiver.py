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
        self.seq_num = 1

    def __enter__(self):
        self.file = open(self.file_name, 'wb')
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.file.close()

    def write(self, payload):
        self.file.write(payload)

    def set_next_seq_num(self):
        self.seq_num = self.next_seq_num()

    def next_seq_num(self):
        return self.seq_num ^ 1

    def ack(self):
        return b'\x00'*4 + struct.pack('>I', self.seq_num)

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
            end_flag = struct.unpack('>B', payload[2:3])[0]
            seq_num = struct.unpack('>I', payload[4:8])[0]
            if (not Receiver.verify_checksum(payload)) or (receiver.next_seq_num() != seq_num):
                logger.writeAck(receiver.next_seq_num(), logHandler.CORRUPTED)
                udp_socket.sendto(receiver.ack(), addr)
                logger.writeAck(receiver.seq_num, logHandler.SEND_ACK_AGAIN)
            else:
                receiver.write(payload[8:])
                logger.writeAck(receiver.next_seq_num(), logHandler.SEND_ACK)
                receiver.set_next_seq_num()
                udp_socket.sendto(receiver.ack(), addr)
                if end_flag == 0:
                    break
    
    logger.writeEnd()