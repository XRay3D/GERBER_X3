#!/usr/bin/env bash

set -ex

# Параметры
REPO_URL="https://github.com/FreeCAD/FreeCAD.git"
BRANCH="v11.2"               # если нужен конкретный branch, иначе убрать
TARGET_DIR="src/Mod/CAM/libarea"     # путь к папке, которую нужно получить
CLONE_DIR="$1/FreeCAD"            # где клонировать

# Клонируем без контента и без checkout
git clone --filter=blob:none --no-checkout -b "$BRANCH" "$REPO_URL" "$CLONE_DIR"
cd "$CLONE_DIR"

# sparse‑checkout: включаем режим cone и указываем нужную папку
git sparse-checkout init --cone
git sparse-checkout set "$TARGET_DIR"

# Перечитаем ветку
git checkout