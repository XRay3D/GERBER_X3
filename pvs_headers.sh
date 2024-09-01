#!/bin/bash
pathSelect=$(pwd)

clear

if [ "$#" -ne 1 ]; then
    for file in $(find $pathSelect -type f -name "*.cpp"); do
        echo $file
        sed -i '/\/\/.*PVS-Studio.*/d' "$file"
    done
else
    for file in $(find $pathSelect -type f -name "*.cpp"); do
        echo $file
        sed -i -e "1i\// This is a personal academic project. Dear PVS-Studio, please check it." \
        -e "1i\// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com" "$file"
    done
fi


