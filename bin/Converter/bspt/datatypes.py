from Converter.helpers import *


UniquePointLength = 16


class UniquePoint():
    size = 16

    def __init__(self, raw_data=None):
        if raw_data is None:
            self.x = 0
            self.y = 0
            self.z = 0
            self.w = 0
        else:
            assert(len(raw_data) == UniquePointLength)
            self.x = bytes_to_fixed(raw_data[0:4])
            self.y = bytes_to_fixed(raw_data[4:8])
            self.z = bytes_to_fixed(raw_data[8:12])
            self.w = bytes_to_fixed(raw_data[12:16])

    def __repr__(self):
        tup = (self.x, self.y, self.z, self.w)
        return "UniquePoint(%f %f %f %f)" % tup

    def serialize(self):
        return self.as_list_4()

    def as_list_3(self):
        return [self.x, self.y, self.z]

    def as_list_4(self):
        return [self.x, self.y, self.z, self.w]


NormalRecordLength = 8


class NormalRecord():
    def __init__(self, raw_data):
        assert(len(raw_data) == NormalRecordLength)
        self.normal_index = bytes_to_unsigned_short(raw_data[0:2])
        self.base_point_index = bytes_to_unsigned_short(raw_data[2:4])
        self.color_index = bytes_to_unsigned_short(raw_data[4:6])
        self.visibility_flags = bytes_to_unsigned_short(raw_data[6:8])

    def __repr__(self):
        tup = (self.normal_index, self.base_point_index, self.color_index, self.visibility_flags)
        return "NormalRecord (%d %d %d %d)" % tup

    def serialize(self):
        return [
            self.normal_index,
            self.base_point_index,
            self.color_index,
            self.visibility_flags
        ]


EdgeRecordLength = 4


class EdgeRecord():
    def __init__(self, raw_data):
        assert(len(raw_data) == EdgeRecordLength)
        self.a = bytes_to_unsigned_short(raw_data[0:2])
        self.b = bytes_to_unsigned_short(raw_data[2:4])

    def __repr__(self):
        tup = (self.a, self.b)
        return "EdgeRecord (%d %d)" % tup

    def serialize(self):
        return [self.a, self.b]


PolyRecordLength = 16


class PolyRecord():
    def __init__(self, raw_data):
        assert(len(raw_data) == PolyRecordLength)
        self.first_edge = bytes_to_unsigned_short(raw_data[0:2])
        self.edge_count = bytes_to_unsigned_short(raw_data[2:4])
        self.normal_index = bytes_to_unsigned_short(raw_data[4:6])
        self.front_poly = bytes_to_unsigned_short(raw_data[6:8])
        self.back_poly = bytes_to_unsigned_short(raw_data[8:10])
        self.visibility = bytes_to_unsigned_short(raw_data[10:12])
        self.reserved = bytes_to_unsigned_long(raw_data[12:16])

    def __repr__(self):
        tup = (self.first_edge, self.edge_count, self.normal_index, self.front_poly, self.back_poly, self.visibility)
        return "PolyRecord (%d %d %d %d %d %d)" % tup

    def serialize(self):
        return [
            self.first_edge,
            self.edge_count,
            self.normal_index,
            self.front_poly,
            self.back_poly,
            self.visibility,
            self.reserved
        ]


ColorRecordLength = (32 * 2) + 4


class ColorRecord():
    def __init__(self, raw_data):
        assert(len(raw_data) == ColorRecordLength)
        self.color_long = bytes_to_long(raw_data[0:4])
        # convert to RGBA float color format
        self.color = [
            ((self.color_long >> 8) & 0xff) / 254.0,
            ((self.color_long >> 16) & 0xff) / 254.0,
            ((self.color_long >> 24) & 0xff) / 254.0,
            (self.color_long & 0xff) / 254.0
        ]
        # colorCache[32] (COLORCACHESIZE)
        # this is not useful to most people because
        # it was used to store intermediate shades
        # of a color when shading it with white light
        self.color_cache = [bytes_to_unsigned_short(x)
                            for x in chunks(raw_data[4:ColorRecordLength], 2)]

    def __repr__(self):
        tup = (self.color, self.color_cache)
        return "ColorRecord (%s %s)" % tup

    def serialize(self):
        return self.color
