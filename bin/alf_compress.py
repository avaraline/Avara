#!/usr/bin/env python3

import argparse
import xml.etree.ElementTree as ET


from dumb_round import dumb_round


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('file', type=open)

    args = parser.parse_args()
    last_wallHeight = 0
    did_output_wallHeight_zero = False
    to_remove = []
    root = ET.fromstring(args.file.read())
    for child in root:
        if child.tag == 'set':
            if 'wallHeight' in child.attrib:
                last_wallHeight = float(child.attrib['wallHeight'])
                if last_wallHeight != 0 or did_output_wallHeight_zero:
                    if len(child.attrib) == 1:
                        to_remove.append(child)
                    else:
                        del child.attrib['wallHeight']
                else:
                    did_output_wallHeight_zero = True
        elif child.tag in ['Wall', 'WallSolid', 'FreeSolid', 'WallDoor', 'Field']:
            if 'h' in child.attrib and child.attrib['h'] == '0':
                child.attrib['h'] = dumb_round(last_wallHeight)
    for child in to_remove:
        root.remove(child)
    print(ET.tostring(root, encoding="unicode"))
