#!/bin/bash
# Run script for Loop Perforation Pass
# Place this script in the benchmarks folder and run it using the name of the file (without the file type)
# e.g. sh run.sh test1

# ACTION NEEDED: Specify your build directory in the project where libLoopPerforationPass.so is located
PATH2LIB="../build/LoopPerforationPass"

# Select the pass
PASS="loop-perforation-pass"

# Delete outputs from previous runs
rm -f *.bc *.ll

# Convert source code to LLVM IR bitcode.
clang -emit-llvm -c ${1}.c -o ${1}.bc

# Run the Loop Perforation Pass
opt -load-pass-plugin="${PATH2LIB}/LoopPerforationPass.so" -passes="${PASS}" ${1}.bc -o ${1}_perforated.bc

# Generate executable from original and transformed bitcode
clang ${1}.bc -o ${1}
clang ${1}_perforated.bc -o ${1}_perforated

# Execute the original and perforated binaries
echo -e "\n=== Execution Output ==="
echo "Original Execution:"
./${1}
echo "Perforated Execution:"
./${1}_perforated

# Optional: Clean up (comment out these lines if you want to keep the files for analysis)
# rm -f *.bc
