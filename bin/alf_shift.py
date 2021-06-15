#!/usr/bin/env python3

import argparse
import re

from pict2alf import dumb_round, METERS_PER_POINT

class Shifter (object):
    def __init__(self, x_offset=0.0, z_offset=0.0):
        self.x_offset = x_offset
        self.z_offset = z_offset

    def shift_x(self, match):
        x = match.group(1)
        new_pt_x = (float(x) / METERS_PER_POINT) + self.x_offset
        new_x = dumb_round(new_pt_x * METERS_PER_POINT)

        out = match.group(0)
        out = out.replace(x, str(new_x))
        return out

    def shift_z(self, match):
        z = match.group(1)
        new_pt_z = (float(z) / METERS_PER_POINT) + self.z_offset
        new_z = dumb_round(new_pt_z * METERS_PER_POINT)

        out = match.group(0)
        out = out.replace(z, str(new_z))
        return out


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('file', type=open)
    parser.add_argument('x_offset', type=float, metavar='x-offset')
    parser.add_argument('z_offset', type=float, metavar='z-offset')

    args = parser.parse_args()
    level_out = args.file.read()
    shifter = Shifter(args.x_offset, args.z_offset)

    # Perform x/z shifts.
    level_out = re.sub(r' x="([0-9\.\-]+)"', shifter.shift_x, level_out)
    level_out = re.sub(r' cx="([0-9\.\-]+)"', shifter.shift_x, level_out)
    level_out = re.sub(r' z="([0-9\.\-]+)"', shifter.shift_z, level_out)
    level_out = re.sub(r' cz="([0-9\.\-]+)"', shifter.shift_z, level_out)

    # Clean up unnecesssary decimal places.
    level_out = re.sub(r'\.0,', ',', level_out)
    level_out = re.sub(r'\.0"', '"', level_out)

    print(level_out)

