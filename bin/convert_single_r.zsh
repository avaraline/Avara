#!/bin/zsh
source /Users/ahalstead/.zshrc
CP=$(which cp)
PYTHON=$(which python3)
PATH="$PATH:/Users/$(whoami)/.local/bin:/opt/homebrew/bin"
path="$(pwd)"
toolspath="$path/bin"
levelspath="$path/levels"
alias ffmpeg="$(which ffmpeg)"
for f in "$@"
do
	echo "$f"
	rname="$f"
	newdirname=${${f:t}:l}
	newdirname=${newdirname// /-}	
	echo $rname
	echo $newdirname	
	# $PYTHON $toolspath/levelset2rsrc.py "$f"
	
	$CP "$rname" "$levelspath/$newdirname"
	$PYTHON $toolspath/rsrc2files.py "$levelspath/$newdirname" "$levelspath/$newdirname"
done

