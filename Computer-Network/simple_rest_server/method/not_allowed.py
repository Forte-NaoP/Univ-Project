from . import method_handler
from ..request import response


class NotAllowed(method_handler.Method):
    @classmethod
    def handler(cls, header, body=None):
        return response.create_response(405)
