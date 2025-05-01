#!/bin/bash

#checking if file is passed as arg

if [ "$#" -lt 1 ]
then
	echo "Pass atleast 1 file as argument !"
	exit 1
fi

for source_name in "$@"; do

	filename="$source_name"
	echo "$filename"
	name="${filename%.*}"

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

	if [ "$filename" -nt "$name.o" ]
	then
		echo "Compiling $filename"
		# Compile to an object file
		if gcc -c "$filename" -o "$name.o"; then
			echo "Successfully compiled to object file: $name.o"
		else
			echo "Error: Failed to compile to object file."
			exit 1
		fi
	else
		echo "$name.o is up to date"
	fi
	
	object_files+=("$name.o")
done

if gcc "${object_files[@]}" -o project_exec;
then
	echo "Linked to project_exec"
else
	echo "Failed to link"
fi




