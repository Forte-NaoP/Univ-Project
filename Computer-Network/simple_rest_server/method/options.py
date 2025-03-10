from . import method_handler
from ..request import response


class Options(method_handler.Method):
    @classmethod
    def handler(cls, header, body=None):
        path = header.get('path')
        if path.startswith('/user'):
            user_id = path[1:].split('/')
            if len(user_id) == 1:
                allow_method = ['GET', 'POST', 'OPTIONS']
                allow_header = ['Content-Type']
                return response.create_response(200, is_preflight=True,
                                                preflight_option={'method': allow_method, 'header': allow_header})
            else:
                allow_method = ['DELETE', 'PUT', 'OPTIONS']
                allow_header = ['Content-Type']
                return response.create_response(200, is_preflight=True,
                                                preflight_option={'method': allow_method, 'header': allow_header})

        return response.create_response(405)
