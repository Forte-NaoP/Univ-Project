import threading
import socket
import signal
import sys

from simple_rest_server.request import request_handler

HOST = '127.0.0.1'
PORT = 1398

server_socket = None

def signal_handler(sig, frame):
    if server_socket:
        server_socket.close()
    sys.exit(0)

if __name__ == '__main__':
    signal.signal(signal.SIGINT, signal_handler)
    
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind((HOST, PORT))
    server_socket.listen()

    while True:
        try:
            client_socket, addr = server_socket.accept()
        except socket.error:
            break

        client_socket.settimeout(5)
        handler = request_handler.RequestHandler(client_socket, addr)
        client_thread = threading.Thread(target=handler.run)
        client_thread.start()
    
    server_socket.close()
