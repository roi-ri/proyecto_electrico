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

install_apt_dependencies() {
    sudo apt update
    sudo apt install -y \
        cmake \
        g++ \
        make \
        pkg-config \
        libwxgtk3.2-dev \
        libgtk-3-dev \
        python3 \
        python3-dev \
        python3-pip \
        dpkg-dev \
        fakeroot \
        zip
}

install_dnf_dependencies() {
    sudo dnf install -y \
        cmake \
        gcc-c++ \
        make \
        pkgconf-pkg-config \
        wxGTK-devel \
        gtk3-devel \
        python3 \
        python3-devel \
        python3-pip \
        rpm-build \
        zip
}

install_pacman_dependencies() {
    sudo pacman -S --needed \
        cmake \
        gcc \
        make \
        pkgconf \
        wxwidgets-gtk3 \
        gtk3 \
        python \
        python-pip \
        zip
}

install_dependencies() {
    if command_exists apt; then
        install_apt_dependencies
        return
    fi
    if command_exists dnf; then
        install_dnf_dependencies
        return
    fi
    if command_exists pacman; then
        install_pacman_dependencies
        return
    fi

    echo "Unsupported Linux package manager."
    echo "Install CMake, a C++ compiler, wxWidgets development files, Python 3 dev files, pip, and zip manually."
    exit 1
}

echo "batView Linux setup"

if [[ "$SKIP_DEPS" -eq 0 ]]; then
    install_dependencies
else
    echo "Skipping dependency installation."
fi

"$SCRIPT_DIR/build_app.sh" "${BUILD_ARGS[@]}"
