#!/bin/zsh

bin/levelset2rsrc.py $1
cp $1.r $2.r
make
bin/rsrc2levelset.py $2.r $2
zip -vr $2.zip $2 