import json

from . import method_handler
from ..request import response, request_handler


class Post(method_handler.Method):
    @classmethod
    def handler(cls, header, body=None):
        path: str = header.get('path')
        if path.startswith('/echo'):
            return response.create_response(200, body)

        elif path.startswith('/user'):
            try:
                user_info = json.loads(body)
                if 'id' not in user_info or 'gender' not in user_info or 'name' not in user_info:
                    return response.create_response(400)

                with request_handler.LOCK:
                    if request_handler.USER.get(user_info.get('id')) is not None:
                        return response.create_response(409)

                    request_handler.USER.update({user_info.get('id'): user_info})
                    return response.create_response(201)

            except json.JSONDecodeError:
                return response.create_response(400)
        else:
            return response.create_response(404)