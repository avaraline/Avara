#!/usr/bin/env python3

import json
import sys


def convert(bsp, out):
    flipped = bsp
    for idx, (x, y, z) in enumerate(bsp["normals"]):
        # Reverse normals
        x = -x if x != 0 else x
        y = -y if y != 0 else y
        z = -z if z != 0 else z
        flipped["normals"][idx] = [x, y, z]
    for idx, p in enumerate(bsp["polys"]):
        # Reverse winding direction
        flipped["polys"][idx]["tris"] = list(reversed(bsp["polys"][idx]["tris"]))
    json.dump(flipped, out)


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("usage: bsp_flip_normals.py <bsp>")
        sys.exit(1)
    with open(sys.argv[1], "r") as f:
        convert(json.load(f), sys.stdout)
        sys.stdout.flush()
