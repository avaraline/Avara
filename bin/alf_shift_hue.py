#!/usr/bin/env python3

import argparse
import re
from typing import Callable
from colorsys import hsv_to_rgb, rgb_to_hsv


def hue_shifter(deg: float) -> Callable[[re.Match], str]:
    def replace_color(m: re.Match) -> str:
        i = int(m.group(1), 16)
        c = [(i >> 16) & 0xff, (i >> 8) & 0xff, i & 0xff]
        c = [n / 255.0 for n in c]
        hsv = list(rgb_to_hsv(*c))
        hsv[0] += (deg / 360.0)
        if hsv[0] < 0:
            hsv[0] += 1
        elif hsv[0] > 1:
            hsv[0] -= 1
        rgb = list(hsv_to_rgb(*hsv))
        rgb = [int(round(n * 255.0)) for n in rgb]
        packed = (rgb[0] << 16) | (rgb[1] << 8) | rgb[2]
        return '"#{:06x}"'.format(packed)
    return replace_color


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("file", type=open)
    parser.add_argument("degrees",  type=float)

    args = parser.parse_args()
    data = args.file.read()
    out = re.sub(r'"#([0-9abcdef]{6})"', hue_shifter(args.degrees), data)
    print(out)
