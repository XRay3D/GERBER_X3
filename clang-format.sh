#!/bin/bash
pathSelect=$(pwd)

codeStyle=./code_style.txt

for folder in ggeasy gte_win plugins static_libs; do
    for ext in h hpp c cpp ino; do
        for pathFile in $(find $pathSelect/$folder -type f -name "*.$ext"); do
            clang-format -i --verbose --style="file:$codeStyle" $pathFile &
        done
    done
done
