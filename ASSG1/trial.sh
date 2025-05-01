#!/bin/bash

for ((i = 0 ; i < 100 ; i++))
do
	if (( i % 2 == 0 ))
	then
		echo "$i" "$(( 100 - i))"
	else
		echo "$i"
	fi
done
