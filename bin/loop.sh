#!/bin/sh
if [ "$#" -lt 1 ]; then
  echo "Usage: $0 file1 [file2...]" >&2
  exit 1
fi

# magic to tell the script to exit when the pipe stops listening
set -eo pipefail

while true ; do cat -u $* ; done
