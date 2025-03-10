import json

from . import method_handler
from ..request import response, request_handler


class Get(method_handler.Method):
    @classmethod
    def handler(cls, header, body=None):
        path = header.get('path')
        if path.startswith('/hi'):
            content = {'message': 'hi'}
            return response.create_response(200, json.dumps(content))
        elif path.startswith('/user'):
            query = path.split('?')[1]
            query_name, query_value = query.split('=')
            if query_name == 'id':
                with request_handler.LOCK:
                    if (user_info := request_handler.USER.get(query_value)) is None:
                        return response.create_response(404)
                    else:
                        return response.create_response(200, json.dumps(user_info))
        else:
            return response.create_response(404)
