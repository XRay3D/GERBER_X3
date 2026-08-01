#!/bin/bash

clear

function git_apply(){
    if [ "$2" = "--" ];then # откатить изменения к индексу
        echo "--" "$1"
        git diff -U0 --ignore-submodules=all | grepdiff -E "$1" --output-matching=hunk | git apply --reverse --unidiff-zero --ignore-space-chang
    elif [ "$2" = "++" ];then # применить изменения к индексу
        echo "++" "$1"
        git diff -U0 --ignore-submodules=all | grepdiff -E "$1" --output-matching=hunk | git apply --cached --unidiff-zero
    fi
}

if [ -n "$2" ];then
    git_apply "$1" $2
elif [ -n "$1" ];then # проверьте, что поиск по регулярному выражению правильно соответствует желаемым изменениям.
    git diff -U0 --ignore-submodules=all  | grepdiff -E "$1" --output-matching=hunk
else
    echo "Использовоние: git_grepdiff 'рег. выражение' (опционально откатить: '--' применить: '++')"
fi

