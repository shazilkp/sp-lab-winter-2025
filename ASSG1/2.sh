#!/bin/sh
for i in $(seq 1 100)
do
	if [ $(expr $i % 11) = 0 ]
	then
		echo "$i"
	fi
done
