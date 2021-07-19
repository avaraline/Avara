#!/usr/bin/env python3

import argparse
import xml.etree.ElementTree as ET


from dumb_round import dumb_round


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('file', type=open)
    parser.add_argument('scaleFactor', type=float)

    args = parser.parse_args()
    root = ET.fromstring(args.file.read())
    for child in root:
        if 'x' in child.attrib or 'z' in child.attrib:
            x = float(child.attrib.get('x', 0))
            z = float(child.attrib.get('z', 0))
            w = float(child.attrib.get('w', 0))
            d = float(child.attrib.get('d', 0))

            child.attrib['x'] = dumb_round(x * args.scaleFactor)
            child.attrib['z'] = dumb_round(z * args.scaleFactor)
            child.attrib['w'] = dumb_round(w * args.scaleFactor)
            child.attrib['d'] = dumb_round(d * args.scaleFactor)
        if 'cx' in child.attrib or 'cz' in child.attrib or 'r' in child.attrib:
            cx = float(child.attrib.get('cx', 0))
            cz = float(child.attrib.get('cz', 0))
            r = float(child.attrib.get('r', 0))
            child.attrib['cx'] = dumb_round(cx * args.scaleFactor)
            child.attrib['cz'] = dumb_round(cz * args.scaleFactor)
            child.attrib['r'] = dumb_round(r * args.scaleFactor)

    print(ET.tostring(root, encoding="unicode"))
