#!/bin/bash

check_prime(){
	if [ $1 -le 1 ]
	then
		return 1
	fi
	
	for i in $(seq 2 $(expr $1 / 2)) 
	do
		if [ $(expr $1 % $i) = 0 ]
		then
			return 1
		fi
	done
	return 0
}

read -a numArray

p_count=0

for num in "${numArray[@]}"
do
	if  check_prime $num
	then
		p_count=`expr $p_count + 1`
	fi
done

echo "$p_count"

