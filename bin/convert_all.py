#!/usr/bin/env python3
from rsrc2files import convert_to_files

import sys
import os

ldir = 'levels'

if __name__ == '__main__':  
    for rsrc_file in os.listdir(ldir):
        rpath = os.path.join(ldir, rsrc_file)
        if os.path.isdir(rpath) or not str(rpath).endswith(".r"):
            continue
        lpath = os.path.join(ldir, rsrc_file.replace(".r", ""))
        print(f"{rpath} -> {lpath}")
        convert_to_files(rpath, lpath)