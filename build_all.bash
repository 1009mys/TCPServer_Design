#!/bin/bash
set -e

MODE=${1:-all}   # clean|debug|release|all

# 기본값 (원하면 환경변수로 오버라이드 가능)
BUILD_SERVER=${BUILD_SERVER:-ON}
BUILD_SERVER_LIBS=${BUILD_SERVER_LIBS:-ON}
BUILD_CLIENT=${BUILD_CLIENT:-ON}

# 인자 파싱: --no-server --no-libs --no-client 등
shift || true
while [[ $# -gt 0 ]]; do
  case "$1" in
    --server)      BUILD_SERVER=ON ;;
    --no-server)   BUILD_SERVER=OFF ;;
    --libs)        BUILD_SERVER_LIBS=ON ;;
    --no-libs)     BUILD_SERVER_LIBS=OFF ;;
    --client)      BUILD_CLIENT=ON ;;
    --no-client)   BUILD_CLIENT=OFF ;;
    *)
      echo "Unknown option: $1"
      echo "Usage:"
      echo "  $0 {clean|debug|release|all} [--no-server] [--no-libs] [--no-client]"
      exit 1
      ;;
  esac
  shift
done

BUILD_DIR=build
DEBUG_DIR=$BUILD_DIR/debug
RELEASE_DIR=$BUILD_DIR/release

common_cmake_args() {
  echo -DBUILD_SERVER=${BUILD_SERVER} \
       -DBUILD_SERVER_LIBS=${BUILD_SERVER_LIBS} \
       -DBUILD_CLIENT=${BUILD_CLIENT}
}

clean() {
    echo "[CLEAN] Removing build directories..."
    rm -rf "$BUILD_DIR"
    echo "[CLEAN] Done."
}

build_debug() {
    echo "[BUILD] Debug (server=${BUILD_SERVER}, libs=${BUILD_SERVER_LIBS}, client=${BUILD_CLIENT})"
    mkdir -p "$DEBUG_DIR"
    cmake -S . -B "$DEBUG_DIR" -DCMAKE_BUILD_TYPE=Debug $(common_cmake_args)
    cmake --build "$DEBUG_DIR"
}

build_release() {
    echo "[BUILD] Release (server=${BUILD_SERVER}, libs=${BUILD_SERVER_LIBS}, client=${BUILD_CLIENT})"
    mkdir -p "$RELEASE_DIR"
    cmake -S . -B "$RELEASE_DIR" -DCMAKE_BUILD_TYPE=Release $(common_cmake_args)
    cmake --build "$RELEASE_DIR"
}

case "$MODE" in
    clean)   clean ;;
    debug)   build_debug ;;
    release) build_release ;;
    all)     build_debug; build_release ;;
    *)
        echo "Usage: $0 {clean|debug|release|all} [--no-server] [--no-libs] [--no-client]"
        exit 1
        ;;
esac
