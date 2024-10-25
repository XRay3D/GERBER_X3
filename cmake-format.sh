#!/bin/bash
pathSelect=$(pwd)

for folder in ggeasy GTE_Win plugins static_libs; do
    for pathFile in $(find $pathSelect/$folder -type f -name 'CMakeLists.txt'); do
#        echo $pathFile
        cmake-format -i --tab-size 4 $pathFile
    done
done
