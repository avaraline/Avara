#!/bin/sh # -x
count=0
if [ "$#" -lt 1 ]; then
    echo "Usage: $0 num" >&2
    echo "  where num = number of players" >&2
  exit 1
fi

count=$1

host=localhost
server_options=(-s)
client_options=(-c $host)
both_options=(-/ "p soundVolume 5")

Avara -n "Player1" "${server_options[@]}" "${both_options[@]}" -/ "p latencyTolerance 1" &
sleep 3

for (( i=2; i < $count; ++i ))
do
  Avara -n "Player$i" "${client_options[@]}" "${both_options[@]}" -/ "p latencyTolerance $i" &
done

# don't put the last player in the background so all can be killed with Ctrl-C
Avara -n "Player$count" "${client_options[@]}" "${both_options[@]}" -/ "p latencyTolerance $count"

# kill all subprocesses
pkill -P $$
