#!/bin/bash
# Usage: sh run.sh filename (without extension)
# Example: sh run.sh test1

# Specify the path to the directory containing the LLVM pass plugin.
PATH2LIB="../build/LoopPerforationPass"
BENCH=${1}.c

# Choose which pass to use. Uncomment the pass you want to use.
#PASS="loop-count-pass"
PASS="loop-perforation-pass"

# Delete outputs from previous runs. Update this if you want to retain some files across runs.
rm -f default.profraw *_prof *_fplicm *.bc *.profdata *_output *.ll *.in *.in.Z

# Compile the C source file to LLVM IR (.bc) and then to human-readable LLVM IR (.ll).
clang -emit-llvm -c ${BENCH} -Xclang -disable-O0-optnone -o ${1}.bc -Wno-deprecated-non-prototype -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk

# Instrument profiler passes. Generates profile data.
opt -passes='pgo-instr-gen,instrprof' ${1}.bc -o ${1}.prof.bc

# Apply the LLVM pass using the opt tool and capture the output.
clang -fprofile-instr-generate ${1}.prof.bc -o ${1}_prof -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk

if [ "${1}" = "anagram" ]; then
./${1}_prof words < input.in > /dev/null 2>&1
elif [ "${1}" = "compress" ];then
./${1}_prof compress.in > /dev/null
else
./${1}_prof > /dev/null
fi

llvm-profdata merge -o ${1}.profdata default.profraw

# The "Profile Guided Optimization Instrumentation-Use" pass attaches the profile data to the bc file.
opt -passes="pgo-instr-use" -o ${1}.profdata.bc -pgo-test-profile-file=${1}.profdata < ${1}.bc

# Optional: Convert the perforated bitcode back to human-readable LLVM IR for inspection.
llvm-dis ${1}.bc -o ${1}.ll
llvm-dis ${1}.profdata.bc -o ${1}.prof.ll

opt --disable-output -load-pass-plugin="${PATH2LIB}/LoopPerforationPass.dylib" -passes="${PASS}" ${1}.profdata.bc

# Compile the original and transformed LLVM IR to machine code executables.
clang ${1}.bc -o "${1}" -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk
clang ${1}.profdata.bc -o "${1}_perforated" -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk

# Execute and compare the outputs of the original and perforated programs.
echo -e "\n=== Execution Output ==="
echo "Original Execution:"
./"${1}"
echo "Perforated Execution:"
./"${1}_perforated"

# Cleanup: Remove this if you want to retain the created files. And you do need to.
rm -f default.profraw *_prof *_fplicm *.bc *.profdata *_output *_perforated
