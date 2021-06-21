#!/usr/bin/env python3

import argparse
import xml.etree.ElementTree as ET


from dumb_round import dumb_round


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('file', type=open)

    args = parser.parse_args()
    lastWallHeight = 0
    to_remove = []
    root = ET.fromstring(args.file.read())
    for child in root:
        if child.tag == 'set':
            if 'wallHeight' in child.attrib:
                lastWallHeight = float(child.attrib.pop('wallHeight'))
                if len(child.attrib) == 0:
                    to_remove.append(child)
        elif child.tag in ['Wall', 'WallSolid', 'FreeSolid', 'WallDoor', 'Field']:
            if 'h' in child.attrib and child.attrib['h'] == '0':
                child.attrib['h'] = dumb_round(lastWallHeight)
    for child in to_remove:
        root.remove(child)
    print(ET.tostring(root, encoding="unicode"))
