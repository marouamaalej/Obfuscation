#!/bin/bash


# RUN ./obfuscate file.c
# Change <Path_to_llvm>

rm *.bc *.ll

fullfilename="/etc/apache2/apache2.conf"
filename=$(basename $1)
fname="${filename%.*}"

fnamebc=$fname'.bc'
fnameO3=$fname'O3'
fnameObfuscated=$fname'Obfuscated'
fnameObfuscatedO3=$fname'ObfuscatedO3'

fnameObfuscatedbc=$fname'Obfuscated.bc'
fnameObfuscatedO3bc=$fname'ObfuscatedO3.bc'

`clang -c -emit-llvm $1 -o $fname.bc`
`opt -load <Path_to_llvm>/build/lib/Obfuscate.so -obfuscate $fname.bc -o $fnameObfuscated.bc`

`llvm-dis $fnameObfuscatedbc`
`llvm-dis $fname.bc`

echo "Checking if the original and obfuscated files are the same based on the generated .ll files..."
if cmp -s $fname.ll $fnameObfuscated.ll; then
    echo "   Same"
else
    echo "   Different, obfuscation performed"
fi

`opt -O3 $fname.bc -o $fnameO3.bc`
`opt -O3 $fnameObfuscated.bc -o $fnameObfuscatedO3.bc`

`llvm-dis $fnameO3.bc`
`llvm-dis $fnameObfuscatedO3.bc`

echo "Checking if the optimized original and optimized obfuscated files are the same based on the generated .ll files..."
if cmp -s $fnameObfuscatedO3.ll $fnameO3.ll; then
    echo "   Same"
else
    echo "   Different, the obfuscation was not eliminated by -O3"
fi

echo "Executable files generated for original and obfuscated codes. Run tests to check the correctness of the obfuscation."
`clang $fnameObfuscatedO3.bc -o $fnameObfuscatedO3`
`clang $fnameO3.bc -o $fname`
