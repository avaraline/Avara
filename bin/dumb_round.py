#!/usr/bin/env python3

import argparse


def dumb_round(num, repeats=6):
    s = str(num)
    if "." not in s:
        return s
    if s.endswith(".0"):
        return s[:-2]
    zeros = "0" * repeats
    nines = "9" * repeats
    if zeros in s:
        return s.split(zeros, 1)[0].rstrip(".")
    if nines in s:
        # How many digits before the nines is how many we want to round to.
        places = len(s.split(nines, 1)[0].split(".")[1]) or None
        return str(round(num, places))
    return s


# How's this for continuous integration.
assert dumb_round(10.00000007) == "10"
assert dumb_round(4.500000022) == "4.5"
assert dumb_round(0.999999987) == "1"
assert dumb_round(0.049999999) == "0.05"


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('num', type=float)

    args = parser.parse_args()
    print(dumb_round(args.num))
