#!/bin/bash
set -e

mkdir -p build
mkdir -p build/debug
mkdir -p build/release

cmake -S . -B build/debug   -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug

cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release
cmake --build build/release