#!/usr/bin/env python3

"""
https://gist.github.com/sfuller/720ea2ef5917ef6c6d20271b0bd56ce5
thanks to Sam Fuller @sfuller

Reference:
http://formats.kaitai.io/apple_single_double/index.html
http://kaiser-edv.de/documents/AppleSingle_AppleDouble.pdf
    above linke is dead, archive:
    https://web.archive.org/web/20180311140826/http://kaiser-edv.de/documents/AppleSingle_AppleDouble.pdf
"""

import enum
from ctypes import BigEndianStructure, c_uint32, c_uint16, c_char
from io import RawIOBase
from typing import List


class AppleDoubleHeader(BigEndianStructure):
    _pack_ = 1
    _fields_ = [
        ('signature',   c_uint32),
        ('version',     c_uint32),
        ('reserved',    c_uint32 * 4),  # Must all be zero
        ('num_entries', c_uint16)
    ]


class AppleDoubleEntry(BigEndianStructure):
    _pack_ = 1
    _fields_ = [
        ('type',        c_uint32),
        ('body_offset', c_uint32),
        ('body_length', c_uint32)
    ]


class EntryType(enum.Enum):
    data_fork = 1
    resource_fork = 2
    real_name = 3  # File name on a file system that supports all the attributes.
    comment = 4
    icon_bw = 5
    icon_color = 6
    file_dates_info = 8  # File creation, modification, access date/timestamps.
    finder_info = 9
    macintosh_file_info = 10
    prodos_file_info = 11
    msdos_file_info = 12
    afp_short_name = 13
    afp_file_info = 14
    afp_directory_id = 15


class Point(BigEndianStructure):
    """Specifies 2D coordinate in QuickDraw grid."""
    _pack_ = 1
    _fields_ = [
        ('x', c_uint16),
        ('y', c_uint16),
    ]


class FinderInfo(BigEndianStructure):
    """From the older Inside Macintosh publication, Volume II page 84 or Volume IV page 104."""
    _pack_ = 1
    _fields_ = [
        ('fdType',     c_char * 4),
        ('fdCreator',  c_char * 4),
        ('fdFlags',    c_uint16),
        ('fdLocation', Point),
        ('fdFolder',   c_uint16)  # File icon's coordinates when displaying this folder.

    ]


class Entry(object):
    def __init__(self):
        self.info = None
        self.data = None


class AppleDouble(object):
    def __init__(self):
        self.header = AppleDoubleHeader()
        self.entries: List[Entry] = []


SIGNATURE_APPLE_SINGLE = 0x00051600
SIGNATURE_APPLE_DOUBLE = 0x00051607


def parse(f: RawIOBase) -> AppleDouble:
    header = AppleDoubleHeader()
    f.readinto(header)

    # Validate signature
    if header.signature not in (SIGNATURE_APPLE_SINGLE, SIGNATURE_APPLE_DOUBLE):
        raise ValueError('Invalid signature')

    entries = []
    for i in range(header.num_entries):
        entry = Entry()
        info = AppleDoubleEntry()
        f.readinto(info)
        entry.info = info
        entries.append(entry)

    for entry in entries:
        info = entry.info
        f.seek(info.body_offset)
        data = f.read(info.body_length)

        if info.type == EntryType.finder_info.value:
            entry.data = FinderInfo.from_buffer_copy(data)
        else:
            entry.data = data

    result = AppleDouble()
    result.header = header
    result.entries = entries
    return result


import sys
if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("usage: appledouble.py <levelset.rsrc>")
        sys.exit(1)
    else:
        with open(sys.argv[1], 'rb') as f:
            double = parse(f)
            count = 0
            for entry in double.entries:
                if entry.info.type == EntryType.resource_fork.value:
                    with open(sys.argv[1] + f"._fork{count}.r", 'wb') as o:
                        o.write(entry.data)
                    count += 1
