#!/bin/bash

filename=$1
base_name=${filename%.*}
preprocess_log="preprocess.log"
assembler_log="assembler.log"
object_log="object.log"
linker_log="linker.log"

echo "Preprocess Log - $(date)" > "$preprocess_log"
echo "Assembly Log - $(date)" > "$assembler_log"
echo "Object Log - $(date)" > "$object_log"

count_issues() {
    local count_warnings=$(grep -c "warning:" "$1")
    local count_errors=$(grep -c "error:" "$1")
    echo "Warnings: $count_warnings"
    echo "Errors: $count_errors"
}


echo "Stage 1: Preprocessing ($filename -> $base_name.i)" | tee -a "$preprocess_log"
gcc -E "$filename" -o "$base_name.i" 2>>"$preprocess_log"
if [ $? -ne 0 ]; then
	echo "Preprocessing failed." | tee -a "$preprocess_log"
	count_issues $preprocess_log
	exit 1
fi

echo "Stage 2: Compiling to Assembly ($base_name.i -> $base_name.s)" | tee -a "$assembler_log"
gcc -S "$base_name.i" -o "$base_name.s" 2>>"$assembler_log"
if [ $? -ne 0 ]; then
    echo "Compilation to Assembly failed." | tee -a "$assembler_log"
    count_issues $assembler_log
    exit 1
fi

# Stage 3: Compilation to Object File
echo "Stage 3: Compiling to Object File ($base_name.s -> $base_name.o)" | tee -a "$object_log"
gcc -c "$base_name.s" -o "$base_name.o" 2>>"$object_log"
if [ $? -ne 0 ]; then
    echo "Compilation to Object File failed." | tee -a "$object_log"
    count_issues $object_log
    exit 1
fi

#Stage 4:Linking 
echo "Stage 4: Linking ($base_name.o-> $base_name)" | tee -a "$linker_log"
gcc "$base_name.o" -o "$base_name" 2>>"$linker_log"
if [ $? -ne 0 ]; then
	echo "Linking failed." | tee -a "$linker_log"
	count_issues $linker_log
	exit 1
fi 

echo "Compilation succesfull"

combined_log="combined.log"
cat "$preprocess_log" "$assembler_log" "$object_log" "$linker_log" > "$combined_log"

count_issues $combined_log

