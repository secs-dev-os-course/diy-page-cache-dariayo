#!/bin/bash

set -e

cd "$(dirname "$0")"

apt update

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

cp -f /usr/lib/linux-tools/6.11.0-9-generic/perf /usr/bin/perf

apt install -y libgtest-dev

cd /usr/src/gtest
cmake . 
make