#!/usr/bin/env python3

import argparse
import numpy as np


def applergb_to_srgb(v):
    c = np.array(v)
    convmat = np.array([[1.06870538834699, 0.024110476735, 0.00173499822713],
                        [-0.07859532843279, 0.96007030899244, 0.02974755969275],
                        [0.00988984558395, 0.01581936633364, 0.96851741859153]])
    linear = np.maximum(convmat.dot(c ** 1.8), np.zeros_like(np.arange(3)))
    linear_section = np.minimum(12.92 * linear, np.array([0.040449936, 0.040449936, 0.040449936]))
    exp_section = 1.055 * (linear ** (1.0 / 2.4)) - 0.055
    res = (255.0 * np.minimum(np.ones_like(np.arange(3)), np.maximum(linear_section, exp_section))).tolist()
    return [int(x) for x in res]


def applergb_to_srgb_u8(v):
    f = [x / 255.0 for x in v]
    return applergb_to_srgb(f)


def color(hex_string):
    if len(hex_string) != 6:
        raise ValueError()
    i = int(hex_string, 16)
    return ((i >> 16) & 0xff, (i >> 8) & 0xff, i & 0xff)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("color", type=color)

    args = parser.parse_args()
    
    srgb_color = tuple(applergb_to_srgb_u8(args.color))
    print("AppleRGB -> sRGB")
    input_hex = hex((args.color[0] << 16) | (args.color[1] << 8) | args.color[2])
    output_hex = hex((srgb_color[0] << 16) | (srgb_color[1] << 8) | srgb_color[2])
    print("{} -> {}".format(input_hex[2:], output_hex[2:]))
    print("{} -> {}".format(args.color, srgb_color))
