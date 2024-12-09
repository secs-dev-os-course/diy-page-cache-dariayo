#!/bin/bash

set -e

# Перейти в директорию скрипта
cd "$(dirname "$0")"

# Обновление пакетов
apt update

# Установка основных инструментов разработки
apt install -y \
    cmake \
    git \
    clang \
    clangd \
    clang-tidy \
    clang-format \
    lldb \
    build-essential \
    g++ \
    time \
    linux-tools-common \
    linux-tools-generic

# Установка perf (обновите путь, если версия отличается)
cp -f /usr/lib/linux-tools/6.11.0-9-generic/perf /usr/bin/perf

# Установка Google Test
apt install -y libgtest-dev

# Сборка Google Test (он поставляется только в виде исходников)
cd /usr/src/gtest
cmake . 
make