#!/bin/bash
set -e

MODE=${1:-all}   # 기본값: all

BUILD_DIR=build
DEBUG_DIR=$BUILD_DIR/debug
RELEASE_DIR=$BUILD_DIR/release

clean() {
    echo "[CLEAN] Removing build directories..."
    rm -rf "$BUILD_DIR"
    echo "[CLEAN] Done."
}

build_debug() {
    echo "[BUILD] Debug"
    mkdir -p "$DEBUG_DIR"
    cmake -S . -B "$DEBUG_DIR" -DCMAKE_BUILD_TYPE=Debug
    cmake --build "$DEBUG_DIR"
}

build_release() {
    echo "[BUILD] Release"
    mkdir -p "$RELEASE_DIR"
    cmake -S . -B "$RELEASE_DIR" -DCMAKE_BUILD_TYPE=Release
    cmake --build "$RELEASE_DIR"
}

case "$MODE" in
    clean)
        clean
        ;;
    debug)
        build_debug
        ;;
    release)
        build_release
        ;;
    all)
        build_debug
        build_release
        ;;
    *)
        echo "Usage: $0 {clean|debug|release|all}"
        exit 1
        ;;
esac
