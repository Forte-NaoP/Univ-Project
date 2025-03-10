from . import method_handler
from ..request import response, request_handler


class Delete(method_handler.Method):
    @classmethod
    def handler(cls, header, body=None):
        path = header.get('path')
        if path.startswith('/user'):
            user_id = path.rsplit('/', 1)[1].split('?')[0]
            with request_handler.LOCK:
                if request_handler.USER.pop(user_id, None) is None:
                    return response.create_response(404)
                else:
                    return response.create_response(200, None)
