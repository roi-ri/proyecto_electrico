#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SKIP_DEPS=0
BUILD_ARGS=()

for arg in "$@"; do
    case "$arg" in
        --skip-deps)
            SKIP_DEPS=1
            ;;
        *)
            BUILD_ARGS+=("$arg")
            ;;
    esac
done

command_exists() {
    command -v "$1" >/dev/null 2>&1
}

install_dependencies() {
    if ! xcode-select -p >/dev/null 2>&1; then
        echo "Installing Apple command line tools. Run this script again after the installer finishes."
        xcode-select --install
        exit 1
    fi

    if ! command_exists brew; then
        echo "Homebrew is required to install CMake, wxWidgets, and Python automatically."
        echo "Install it from https://brew.sh and run this script again."
        exit 1
    fi

    brew update
    brew install cmake wxwidgets python pkg-config
}

echo "batView macOS setup"

if [[ "$SKIP_DEPS" -eq 0 ]]; then
    install_dependencies
else
    echo "Skipping dependency installation."
fi

"$SCRIPT_DIR/build_app.sh" "${BUILD_ARGS[@]}"
