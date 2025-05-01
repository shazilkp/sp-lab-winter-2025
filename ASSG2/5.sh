#!/bin/bash

filename=$1
name=${filename%.*}

#gcc -o libarith.so -fpic -shared libarith.c

#gcc -c "$filename" -o "$name.o"
#gcc -o "$name" "$name.o" -Wl,-rpath=$(pwd) -larith -L.

gcc -o "$name" "$filename" -Wl,-rpath=$(pwd) -larith -L.
