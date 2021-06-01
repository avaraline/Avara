#!/usr/bin/env python3

import sys
import struct

data = open(sys.argv[1], 'rb').read()

if __name__ == "__main__":
	if len(sys.argv) != 2:
		print("Usage: text.py <TEXT.r>")
		sys.exit(1)
	else:
		with open(sys.argv[1], "rb") as input_file:
			print(input_file.read().decode("macroman").replace("\r", "\n"))