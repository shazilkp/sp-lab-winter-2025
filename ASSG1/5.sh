#!/bin/bash

read -a stringArray

for i in ${stringArray[@]}
do
	if [ "$(echo $i | rev)" = "$i" ]
	then
		echo $i
	fi
done
