#!/usr/bin/env python3

import os
import sys

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("usage: levelset2rsrc.py <levelset>")
        sys.exit(1)
    else:
        with open(os.path.join(sys.argv[1], "..namedfork", "rsrc"), "rb") as src_file:
            with open(sys.argv[1] + ".r", "wb") as dest_file:
                dest_file.write(src_file.read())
