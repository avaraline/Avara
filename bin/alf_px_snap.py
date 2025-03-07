#!/usr/bin/env python3

import argparse
import re
import xml.etree.ElementTree as ET


from dumb_round import dumb_round
from pict2alf import METERS_PER_POINT


def snap(el):
    """
    Snap x/z coords to original game PICT limitations.
    """
    x = float(child.attrib.get('x', 0))
    z = float(child.attrib.get('z', 0))
    w = float(child.attrib.get('w', 0))
    d = float(child.attrib.get('d', 0))

    left = x - w/2
    right = left + w
    top = z - d/2
    bottom = top + d

    left = round(left / METERS_PER_POINT) * METERS_PER_POINT
    right = round(right / METERS_PER_POINT) * METERS_PER_POINT
    top = round(top / METERS_PER_POINT) * METERS_PER_POINT
    bottom = round(bottom / METERS_PER_POINT) * METERS_PER_POINT

    x = dumb_round((right + left) / 2)
    z = dumb_round((bottom + top) / 2)
    w = dumb_round(right - left)
    d = dumb_round(bottom - top)

    el.attrib['x'] = str(x)
    el.attrib['z'] = str(z)
    el.attrib['w'] = str(w)
    el.attrib['d'] = str(d)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('file', type=open)

    args = parser.parse_args()
    root = ET.fromstring(args.file.read())
    for child in root:
        if child.tag in ['Wall', 'Ramp', 'YonBox']:
            snap(child)

    print(ET.tostring(root, encoding="unicode"))
