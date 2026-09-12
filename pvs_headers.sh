#!/bin/bash
# Проставить/снять комментарий PVS-Studio для бесплатной opensource-лицензии.
# Без аргументов -- удалить, с любым аргументом -- добавить.
# Обрабатываются только файлы, отслеживаемые git (third_party и build не трогаются).
cd "$(git rev-parse --show-toplevel)" || exit 1

files=$(git ls-files | grep -E '\.(cpp|cc|cxx|c)$')

if [ "$#" -eq 0 ]; then
    for file in $files; do
        sed -i '/^\/\/.*PVS-Studio.*/d' "$file"
    done
else
    for file in $files; do
        sed -i -e '1i\// This is an open source non-commercial project. Dear PVS-Studio, please check it.' \
               -e '1i\// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com' "$file"
    done
fi
