import sys, os, threading, time, socket, struct

sys.path.append(os.path.dirname(os.path.abspath(os.path.dirname(__file__))))
from logHandler import logHandler
import PASender

# ----------------------------------------------
# | end_flag(1) |  seq_num (1) |  ...payload   |
# ----------------------------------------------

MAX_PAYLOAD_SIZE = 1460
MAX_READ_SIZE = MAX_PAYLOAD_SIZE - 2

PORT_NUMBER = 10090


class Payload():
    
    def __init__(self, file_name):
        self.file_name = file_name
        self.file_size = os.path.getsize(file_name)
        self.seq_num = 0

    def __enter__(self):
        self.file = open(self.file_name, 'rb')
        return self
    
    def __exit__(self, exc_type, exc_value, traceback):
        self.file.close()

    def get_payload(self):
        while (payload := self.file.read(MAX_READ_SIZE)):
            end_flag = struct.pack('>B', 0 if self.file.tell() == self.file_size else 1)
            seq_num = struct.pack('>B', self.seq_num)

            payload =  end_flag + seq_num + payload
            yield payload

    def next_seq_num(self):
        self.seq_num ^= 1
    

if __name__ == "__main__":
    if len(sys.argv) != 5:
        print("Usage: python3 sender.py <receiver_ip> <window_size> <source_file> <log_file>")
        exit(1)
        
    receiver_ip = sys.argv[1]
    window_size = int(sys.argv[2])
    source_file = sys.argv[3]
    log_file = sys.argv[4]

    udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    udp_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sender = PASender.PASender(udp_socket)
    logger = logHandler()
    
    logger.startLogging(log_file)
    with Payload(source_file) as loader:
        for payload in loader.get_payload():
            sender.sendto_bytes(payload, (receiver_ip, PORT_NUMBER))
            logger.writePkt(loader.seq_num, logHandler.SEND_DATA)
            # loader.next_seq_num()

    logger.writeEnd()