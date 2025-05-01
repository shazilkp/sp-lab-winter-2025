#!/bin/bash

filename=$1
name=${filename%.*}
flag=$2

if [ "$#" -ne 2 ]; then
	echo "Please enter in format : $0 <source_file.c> <compilation_stage_flag (-E, -S, -c)>"
	exit 1
fi

if [[ "$flag" != "-E" && "$flag" != "-S" && "$flag" != "-c" ]]; then
	echo "Error: Invalid compilation stage flag '$flag'. Valid flags are -E, -S, -c."
	exit 1
fi

#check if extension is c
if [[ $filename != *.c ]]
then
	echo "Please enter a .c file"
	exit 1
fi

#checking if file exist

if [ ! -e $filename ]
then
	echo "$filename doesnt exist"
fi


if [ "$flag" == "-E" ]
then
	if gcc -E "$filename" -o "$name.i";
	then
		echo "Generated preprocessed file : $name.i"
	else
		echo "Error: Failed to generate preprocessed file"
		exit 1
	fi 
fi

if [ "$flag" == "-S" ]
then
	if gcc -S "$filename" -o "$name.s";
	then
		echo "Generated assembly file : $name.s"
	else
		echo "Error: Failed to generate assembly file"
		exit 1
	fi 
fi

if [ "$flag" == "-c" ]
then
	if gcc -c "$filename" -o "$name.s";
	then
		echo "Generated object file : $name.s"
	else
		echo "Error: Failed to generate object file"
		exit 1
	fi 
fi



