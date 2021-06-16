#!/usr/bin/env python3

import argparse
import xml.etree.ElementTree as ET


from pict2alf import dumb_round


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('file', type=open)

    args = parser.parse_args()
    root = ET.fromstring(args.file.read())
    for child in root:
        if 'x' in child.attrib or 'z' in child.attrib:
            x = float(child.attrib.get('x', 0))
            z = float(child.attrib.get('z', 0))
            w = float(child.attrib.get('w', 0))
            d = float(child.attrib.get('d', 0))

            center_x = x + (w / 2)
            center_z = z + (d / 2)

            child.attrib['x'] = str(dumb_round(center_z - (d / 2)))
            child.attrib['z'] = str(dumb_round((-1 * center_x) - (w / 2)))
            child.attrib['w'] = str(d)
            child.attrib['d'] = str(w)
        if 'cx' in child.attrib or 'cz' in child.attrib or 'angle' in child.attrib:
            cx = float(child.attrib.get('cx', 0))
            cz = float(child.attrib.get('cz', 0))
            angle = float(child.attrib.get('angle', 0))
            child.attrib['cx'] = str(cz)
            child.attrib['cz'] = str(-1 * cx)
            child.attrib['angle'] = str((angle - 90) % 360)

    print(ET.tostring(root, encoding="unicode"))
