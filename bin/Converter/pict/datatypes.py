from Converter.helpers import *


class Rect (object):
    def __init__(self, bytes):
        self.src = Point(bytes[0:4])
        self.dst = Point(bytes[4:8])


class Point (object):
    def __init__(self, bytes):
        self.x = bytes_to_short(bytes[0:2])
        self.y = bytes_to_short(bytes[2:4])
