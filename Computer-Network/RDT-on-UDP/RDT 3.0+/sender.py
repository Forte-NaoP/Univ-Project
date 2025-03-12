import sys, os, threading, time, socket, struct
from collections import deque

sys.path.append(os.path.dirname(os.path.abspath(os.path.dirname(__file__))))
from logHandler import logHandler
import PASender


MAX_PAYLOAD_SIZE = 1460
MAX_READ_SIZE = MAX_PAYLOAD_SIZE - 8

# ----------------------------------------------------------------
# | checkSum (2)| end_flag(1) |  window_Size (1) |   seq_num (4) |
# ----------------------------------------------------------------

class Payload:
    
    def __init__(self, file_name, window_size=1):
        self.file_name = file_name
        self.file_size = os.path.getsize(file_name)
        self.seq_num = 0
        self.wsize = window_size
        self.base = window_size * 2
        self.window = deque(maxlen=window_size)
        self.window_mark = [False for _ in range(self.base)]

    def __enter__(self):
        self.file = open(self.file_name, 'rb')
        self.chunk = self.__get_chunk()
        self.__init_window()
        return self
    
    def __exit__(self, exc_type, exc_value, traceback):
        self.file.close()

    def __get_chunk(self):
        while (payload := self.file.read(MAX_READ_SIZE)):
            end_flag = struct.pack('>B', 0 if self.file.tell() == self.file_size else 1)
            yield end_flag, payload

    def __get_payload(self, seq_num):
        try:
            end_flag, payload = next(self.chunk)
            payload = end_flag + struct.pack('>B', self.wsize) + struct.pack('>I', seq_num) + payload
            checksum = self.checksum(payload)
            return struct.pack('>H', checksum) + payload
        except StopIteration:
            return None

    def __init_window(self):
        for i in range(self.wsize):
            self.window.append((i, self.__get_payload(i)))

    def mark_seq_num(self, seq_num):
        self.window_mark[seq_num % self.base] = True
    
    def reset_mark(self, seq_num):
        self.window_mark[seq_num % self.base] = False

    def next_seq_num(self):
        self.seq_num = (self.seq_num + 1) % self.base

    def move_window(self):
        self.next_seq_num()
        self.window.popleft()
        seq_num = (self.seq_num + self.wsize - 1) % self.base
        if payload := self.__get_payload(seq_num):
            self.window.append((seq_num, payload))
            return True
        return False

    def is_in_window(self, seq_num):
        if self.seq_num + self.wsize <= self.base:
            return self.seq_num <= seq_num < self.seq_num + self.wsize
        else:
            return self.seq_num <= seq_num or seq_num < (self.seq_num + self.wsize) % self.base

    @staticmethod
    def checksum(payload):
        if len(payload) % 2 == 1:
            payload += b'\x00'
        
        checksum = 0
        for i in range(0, len(payload), 2):
            checksum += int.from_bytes(payload[i:i+2], 'big')
            checksum = (checksum >> 16) + (checksum & 0xffff)
        
        return ~checksum & 0xffff



PORT_NUMBER = 10090

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
    udp_socket.settimeout(0.01)
    sender = PASender.PASender(udp_socket, 'config.txt')
    logger = logHandler()


    logger.startLogging(log_file)
    with Payload(source_file, window_size) as loader:

        for seq_num, payload in loader.window:
            sender.sendto_bytes(payload, (receiver_ip, PORT_NUMBER))
            logger.writePkt(seq_num, logHandler.SEND_DATA)

        while len(loader.window) != 0:
            try:
                while (ack := udp_socket.recvfrom(8)):
                    ack, addr = ack
                    seq_num = struct.unpack('>I', ack[4:])[0]
                    if loader.is_in_window(seq_num):
                        if seq_num == loader.seq_num:
                            logger.writePkt(seq_num, logHandler.SUCCESS_ACK)                          
                        else:
                            logger.writePkt(seq_num, logHandler.WRONG_SEQ_NUM)
                            logger.writePkt(seq_num, logHandler.SUCCESS_ACK_MARK)

                        loader.mark_seq_num(seq_num)
                        cur_seq_num = loader.seq_num
                        for i in range(loader.wsize):
                            if not loader.window_mark[(i+cur_seq_num) % loader.base]:
                                break
                            if loader.move_window():
                                if len(loader.window) == 0:
                                    break
                                sender.sendto_bytes(loader.window[-1][1], (receiver_ip, PORT_NUMBER))
                                logger.writePkt(loader.window[-1][0], logHandler.SEND_DATA)
                                loader.reset_mark((i+cur_seq_num) % loader.base)
                        break
            except socket.timeout:
                logger.writeTimeout(loader.seq_num)
                sender.sendto_bytes(loader.window[0][1], (receiver_ip, PORT_NUMBER))
                logger.writePkt(loader.seq_num, logHandler.SEND_DATA_AGAIN)
    logger.writeEnd()