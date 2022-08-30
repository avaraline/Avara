#!/usr/bin/env python3

import argparse
import numpy as np


def applergb_to_srgb(v):
    c = np.array(v)
    convmat = np.array([[1.02524, -0.0265452, 0.00130355],
                        [0.0194008, 0.948009, 0.0325904],
                        [-0.00176213, -0.0014345, 1.0032]])
    linear = np.maximum(convmat.dot(c ** 1.8), np.zeros_like(np.arange(3)))
    linear_section = np.minimum(12.92 * linear, np.array([0.0031308, 0.0031308, 0.0031308]))
    exp_section = ((1.13712 * linear) ** (1.0 / 2.4)) - 0.055
    res = (255.0 * np.minimum(np.ones_like(np.arange(3)), np.maximum(linear_section, exp_section))).tolist()
    return [int(x) for x in res]


def applergb_to_srgb_u8(v):
    return applergb_to_srgb([x / 255.0 for x in v])


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
