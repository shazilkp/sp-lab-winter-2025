#!/bin/bash

#checking if file is passed as arg

if [ "$#" -ne 1 ]
then
	echo "Pass a file as argument !"
	exit 1
fi

filename=$1
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

#running preprocessor

if cpp "$filename" > "$name.i";
then
	echo "generated preprocessed file : $name.i"
else
	echo "Error: Failed to generate preprocessed file"
	exit 1
fi 

#assembly
if gcc -S "$name.i" -o "$name.s";
then
	echo "Successfully generated assembly file: $name.s"
else
	echo "Error: Failed to generate assembly file."
	exit 1
fi

# Compile to an object file
if gcc -c "$name.s" -o "$name.o"; then
	echo "Successfully compiled to object file: $name.o"
else
	echo "Error: Failed to compile to object file."
	exit 1
fi

# Link and create the executable
if gcc "$name.o" -o final_exec; then
  echo "Successfully created executable: final_exec"
else
  echo "Error: Failed to create executable."
  exit 1
fi

