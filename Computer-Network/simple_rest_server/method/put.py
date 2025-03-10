import json

from . import method_handler
from ..request import response, request_handler


class Put(method_handler.Method):
    @classmethod
    def handler(cls, header, body=None):
        path = header.get('path')
        if path.startswith('/user'):
            user_id = path.rsplit('/', 1)[1].split('?')[0]
            update_info = json.loads(body)
            with request_handler.LOCK:
                if (user_info := request_handler.USER.get(user_id)) is None:
                    return response.create_response(404)
                else:
                    if 'id' in update_info or 'gender' in update_info or 'name' not in update_info:
                        return response.create_response(400)
                    if user_info.get('name') == update_info.get('name'):
                        return response.create_response(422)

                    user_info.update({'name': update_info.get('name')})
                    request_handler.USER.update({user_id: user_info})
                    return response.create_response(200)