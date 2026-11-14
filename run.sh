#!/usr/bin/env bash
# Build, test and run the order book.
#
# Usage:
#   ./run.sh            build, run tests, then run the app (debug + sanitizers)
#   ./run.sh build      build only
#   ./run.sh test       build, then run tests
#   ./run.sh release    optimised build without sanitizers (for benchmarks), then run the app
#   ./run.sh clean      delete all build folders

# -e: stop at the first command that fails
# -u: using an unset variable is an error (catches typos)
# -o pipefail: a pipeline fails if any command in it fails, not just the last one
set -euo pipefail

# Always run from the repo root, wherever the script is called from.
# $0 is the path to this script; dirname strips the file name.
cd "$(dirname "$0")"

# ${1:-all} means: the first argument, or "all" if none was given.
mode="${1:-all}"

# Create the build folder's makefiles if they don't exist yet.
# $1 = build folder, any further arguments are passed on to cmake.
configure() {
    local dir="$1"
    shift                        # drop $1, so "$@" is now the remaining arguments
    if [[ ! -d "$dir" ]]; then   # -d tests whether a directory exists
        cmake -S . -B "$dir" "$@"
    fi
}

build() {
    configure build
    cmake --build build -j
}

run_tests() {
    ctest --test-dir build --output-on-failure
}

case "$mode" in
    all)
        build
        run_tests
        ./build/orderbook_app
        ;;
    build)
        build
        ;;
    test)
        build
        run_tests
        ;;
    release)
        configure build-release -DCMAKE_BUILD_TYPE=Release -DORDERBOOK_SANITIZE=OFF
        cmake --build build-release -j
        ./build-release/orderbook_app
        ;;
    clean)
        rm -rf build build-release
        ;;
    *)
        # >&2 sends the message to stderr instead of stdout
        echo "Unknown mode: $mode (use: all, build, test, release, clean)" >&2
        exit 1
        ;;
esac
