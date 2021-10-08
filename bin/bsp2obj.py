#!/usr/bin/env python3

import json
import sys


def convert(bsp, out):
    for pt in bsp["points"]:
        out.write("v {} {} {}\n".format(*pt))
    for p in bsp["polys"]:
        out.write("vn {} {} {}\n".format(*p["normal"]))
    for idx, p in enumerate(bsp["polys"]):
        for t in p["tris"]:
            out.write(
                "f {a}//{n} {b}//{n} {c}//{n}\n".format(
                    a=t[0] + 1,
                    b=t[1] + 1,
                    c=t[2] + 1,
                    n=idx + 1,
                )
            )


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("usage: bsp2obj.py <bsp>")
        sys.exit(1)
    with open(sys.argv[1], "r") as f:
        convert(json.load(f), sys.stdout)
        sys.stdout.flush()
