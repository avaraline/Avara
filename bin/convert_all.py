#!/usr/bin/env python3
from rsrc2files import convert_to_files
import multiprocessing

import sys
import os

ldir = "levels"


if __name__ == "__main__":
    sets = sys.argv[1:]
    paths = []
    for rsrc_file in os.listdir(ldir):
        if sets and rsrc_file not in sets:
            continue
        rpath = os.path.join(ldir, rsrc_file)
        if os.path.isdir(rpath) or not str(rpath).endswith(".r"):
            continue
        lpath = os.path.join(ldir, rsrc_file.replace(".r", ""))
        paths.append((rpath, lpath))
    with multiprocessing.Pool() as pool:
        pool.starmap(convert_to_files, paths)
