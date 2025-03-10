import sys
import os
import threading
import time
import socket
import json
from abc import *


class Method(metaclass=ABCMeta):
    @classmethod
    def handler(cls, header, body=None):
        pass
