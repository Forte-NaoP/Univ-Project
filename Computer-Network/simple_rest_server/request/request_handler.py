import socket
import threading
from simple_rest_server.method import get, post, options, put, delete, not_allowed

USER = dict()
LOCK = threading.Lock()

def split(header):
    k, v = header.split(':', 1)
    return k, v.strip()

def packet_parser(packet: str):
    try:
        header, content = packet.split('\r\n\r\n')
        header_lines = header.split('\r\n')

        method, path, version = header_lines[0].split(' ')

        header_dict = dict(map(split, header_lines[1:]))
        header_dict.update({'method': method, 'path': path, 'version': version})
        return header_dict, content
    except ValueError as e:
        return {}, ''


class RequestHandler:
    def __init__(self, client_socket, addr):
        self.socket = client_socket
        self.addr = addr
        self.handler = {
            'GET': get.Get.handler,
            'POST': post.Post.handler,
            'OPTIONS': options.Options.handler,
            'PUT': put.Put.handler,
            'DELETE': delete.Delete.handler,
        }

    def run(self):
        try:
            while (packet := self.socket.recv(1024)) is not None:
                packet = packet.decode()
                header, body = packet_parser(packet)

                self.socket.sendall(
                    self.handler.get(header.get('method'), not_allowed.NotAllowed.handler)(header, body)
                )

        except (socket.timeout, ConnectionAbortedError):
            self.socket.close()
            return


