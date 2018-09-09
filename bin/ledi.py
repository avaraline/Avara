#!/usr/bin/env python3

import sys
import struct


def CharConst(n):
    return chr((n & 0xFF000000) >> 24) + chr((n & 0x00FF0000) >> 16) + chr((n & 0x0000FF00) >> 8) + chr(n & 0x000000FF)


def even_pstring(data, pos):
    length = data[pos]
    b = data[pos+1:pos+1+length]
    if length % 2 == 0:
        length += 1
    return b.decode('macroman'), pos + 1 + length


data = open(sys.argv[1], 'rb').read()
set_tag, num_levels = struct.unpack('!LH', data[:6])
print('{} - {} levels\n'.format(CharConst(set_tag), num_levels))
pos = 6
for i in range(num_levels):
    level_tag = struct.unpack('!L', data[pos:pos+4])[0]
    pos += 4
    name, pos = even_pstring(data, pos)
    intro, pos = even_pstring(data, pos)
    rsrc_name, pos = even_pstring(data, pos)
    print('  {} - {} ({})'.format(CharConst(level_tag), name, rsrc_name))
    print('  ' + '\n  '.join(line for line in intro.split('\r') if line.strip()) + '\n')
    enables, from_file, rsvd, count_enables = struct.unpack('!hHLh', data[pos:pos+10])
    pos += 10
    count_enables = min(count_enables, 64)
    pos += 4 * count_enables
