#!/usr/bin/env python3

import argparse
import xml.etree.ElementTree as ET


from dumb_round import dumb_round


VESTIGIAL_ATTRS = {
    'Walker': ['fill', 'frame', 'r'],
    'Sphere': ['r'],
    'Hologram': ['r'],
    'Solid': ['r'],
    'WallSolid': ['angle', 'cx', 'cz', 'r'],
    'FreeSolid': ['r'],
    'Dome': [],
    'Ramp': ['cx', 'cz', 'r'],
    'TriPyramid': ['r'],
    'Door': ['r'],
    'Door2': ['r'],
    'WallDoor': ['angle', 'cx', 'cz', 'r'],
    'Area': ['fill', 'frame', 'angle'],
    'Text': ['fill', 'frame', 'angle'],
    'Field': ['r'],
    'Goody': ['r'],
    'Switch': ['r'],
    'Guard': ['r'],
    'GroundColor': ['frame', 'angle', 'cx', 'cz', 'r'],
    'SkyColor': ['angle', 'cx', 'cz', 'r'],
    'Incarnator': ['fill', 'frame', 'r'],
    'Teleporter': ['r'],
    'Pill': ['r'],
    'Ball': ['r'],
    'Goal': ['r'],
    'Mine': ['r'],
    'Parasite': ['r'],
    'Ufo': ['r'],
    'Sound': ['fill', 'frame', 'r'],
    'Timer': ['fill', 'frame', 'angle', 'cx', 'cz', 'r'],
    'Delay': ['fill', 'frame', 'angle', 'cx', 'cz', 'r'],
    'Counter': ['fill', 'frame', 'angle', 'cx', 'cz', 'r'],
    'And': ['fill', 'frame', 'angle', 'cx', 'cz', 'r'],
    'Distributor': ['fill', 'frame', 'angle', 'cx', 'cz', 'r'],
    'Base': ['fill', 'frame', 'angle', 'r'],
    'YonBox': ['fill', 'frame'],
    'YonSphere': ['fill', 'frame'],
}


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('file', type=open)

    args = parser.parse_args()
    last_wallHeight = 3 # This is the default value in rsrc/default.avarascript
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
        if child.tag in VESTIGIAL_ATTRS.keys():
            child.attrib = {k: v for k, v in child.attrib.items()
                            if k not in VESTIGIAL_ATTRS[child.tag]}
    for child in to_remove:
        root.remove(child)
    print(ET.tostring(root, encoding="unicode"))
