#!/bin/sh # -x

host=localhost
if [ "$1" == "-x" ]; then
    host=`curl locallhost.com 2> /dev/null`
    shift # $2 -> $1
fi

if [ "$#" -lt 1 ] || [ $1 -lt 2 ]; then
    echo "Usage: $0 [-x] num" >&2
    echo "  num = number of players (min. 2)" >&2
    echo "  -x use external IP when connecting to server" >&2
    exit 1
fi

count=$1

server_options=(-s)
client_options=(-c $host)
both_options=(-/ "p soundVolume 5")

Avara -n "Player1" "${server_options[@]}" "${both_options[@]}" &
sleep 3

for (( i=2; i < $count; ++i ))
do
  Avara -n "Player$i" "${client_options[@]}" "${both_options[@]}" &
  sleep 1
done

# don't put the last player in the background so all can be killed with Ctrl-C
Avara -n "Player$count" "${client_options[@]}" "${both_options[@]}"

# kill all subprocesses
pkill -P $$
