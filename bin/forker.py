#!/usr/bin/env python3
# Parses out classic MacOS resource fork data into files
# See https://developer.apple.com/library/archive/documentation/mac/pdf/MoreMacintoshToolbox.pdf (1-121)

import os
import re
import struct
import sys


def CharConst(n):
    return chr((n & 0xFF000000) >> 24) + chr((n & 0x00FF0000) >> 16) + chr((n & 0x0000FF00) >> 8) + chr(n & 0x000000FF)


if len(sys.argv) != 3:
    print('Usage: forker.py <rsrc_file> <output_dir>')
    sys.exit(1)


os.makedirs(sys.argv[2], exist_ok=True)
data = open(sys.argv[1], 'rb').read()
# resource header
data_offset, map_offset, data_len, map_len = struct.unpack('!4L', data[:16])
# resource map
fork_attrs, type_list_offset, name_list_offset, num_types = struct.unpack('!4H', data[map_offset+22:map_offset+30])
offset = map_offset + type_list_offset + 2 # already read num_types above
for i in range(num_types + 1):
    # type list entry
    rsrc_type, num_resources, rsrc_list_offset = struct.unpack('!LHH', data[offset:offset+8])
    rsrc_offset = map_offset + type_list_offset + rsrc_list_offset
    for k in range(num_resources + 1):
        # reference list entry
        rsrc_id, name_offset, rsrc_info = struct.unpack('!HhL', data[rsrc_offset:rsrc_offset+8])
        rsrc_attrs = rsrc_info >> 24
        rsrc_data_offset = data_offset + (rsrc_info & 0x00FFFFFF)
        rsrc_data_len = struct.unpack('!L', data[rsrc_data_offset:rsrc_data_offset+4])[0]
        rsrc_data = data[rsrc_data_offset+4:rsrc_data_offset+4+rsrc_data_len]
        rsrc_dir = os.path.join(sys.argv[2], CharConst(rsrc_type))
        if name_offset >= 0:
            rsrc_name_offset = map_offset + name_list_offset + name_offset
            namelen = data[rsrc_name_offset]
            name = data[rsrc_name_offset+1:rsrc_name_offset+1+namelen].decode('macroman')
        else:
            name = ''
        filename = '{}-{}'.format(rsrc_id, name) if name else str(rsrc_id)
        filename = re.sub(r'[^a-z0-9\-\.]', '', filename, flags=re.I)
        filename = str(rsrc_id) # remove this line to write out ID-NAME files
        rsrc_path = os.path.join(rsrc_dir, filename)
        print('{} #{} [{}] ({} bytes) --> {}'.format(CharConst(rsrc_type), rsrc_id, name, rsrc_data_len, rsrc_path))
        os.makedirs(rsrc_dir, exist_ok=True)
        with open(rsrc_path, 'wb') as f:
            f.write(rsrc_data)
        rsrc_offset += 12
    offset += 8
