#!/usr/bin/env bash
# create_ubuntu_package.sh — Build and package nuBASIC as a Debian (.deb) package
#
# Usage:
#   ./create_ubuntu_package.sh [--build-dir <path>] [--console-only] [--jobs <n>]
#                              [--no-install-deps] [--help]
#
# Options:
#   --build-dir <path>   Build directory (default: ./build-ubuntu-deb)
#   --console-only       Skip GTK2 IDE; build interpreter only
#   --jobs <n>           Parallel build jobs (default: nproc)
#   --no-install-deps    Skip apt-get install (assume deps already present)
#
# The resulting .deb is placed in <build-dir> and its path is printed at the end.

set -euo pipefail

# ── defaults ──────────────────────────────────────────────────────────────────
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build-ubuntu-deb"
CONSOLE_ONLY=0
INSTALL_DEPS=1
JOBS="$(nproc 2>/dev/null || echo 4)"
INSTALL_PREFIX="/usr"

# ── argument parsing ──────────────────────────────────────────────────────────
while [[ $# -gt 0 ]]; do
    case "$1" in
        --build-dir)   BUILD_DIR="$2";  shift 2 ;;
        --console-only) CONSOLE_ONLY=1; shift   ;;
        --jobs)        JOBS="$2";       shift 2 ;;
        --no-install-deps) INSTALL_DEPS=0; shift ;;
        --help|-h)
            sed -n '2,12p' "$0" | sed 's/^# \?//'
            exit 0 ;;
        *) echo "Unknown option: $1" >&2; exit 1 ;;
    esac
done

# ── helpers ───────────────────────────────────────────────────────────────────
info()  { printf '\033[1;34m[INFO]\033[0m  %s\n' "$*"; }
ok()    { printf '\033[1;32m[ OK ]\033[0m  %s\n' "$*"; }
warn()  { printf '\033[1;33m[WARN]\033[0m  %s\n' "$*"; }
die()   { printf '\033[1;31m[ERR ]\033[0m  %s\n' "$*" >&2; exit 1; }

require_cmd() {
    command -v "$1" >/dev/null 2>&1 || die "Required command not found: $1"
}

# ── platform check ────────────────────────────────────────────────────────────
if [[ "$(uname -s)" != "Linux" ]]; then
    die "This script targets Ubuntu/Debian Linux. Detected: $(uname -s)"
fi

if ! command -v apt-get >/dev/null 2>&1; then
    die "apt-get not found — this script requires an Ubuntu/Debian system."
fi

DISTRO="$(lsb_release -si 2>/dev/null || echo unknown)"
RELEASE="$(lsb_release -sr 2>/dev/null || echo unknown)"
info "Detected distribution: ${DISTRO} ${RELEASE}"

# ── install build dependencies ────────────────────────────────────────────────
BASE_DEPS=(
    build-essential
    cmake
    ninja-build
    pkg-config
    libffi-dev
    dpkg-dev
)

GTK_DEPS=(
    libgtk2.0-dev
    libglib2.0-dev
    libpango1.0-dev
    libatk1.0-dev
    libcairo2-dev
    libx11-dev
)

if [[ "$INSTALL_DEPS" -eq 1 ]]; then
    info "Installing base build dependencies..."
    sudo apt-get update -qq
    sudo apt-get install -y --no-install-recommends "${BASE_DEPS[@]}"

    if [[ "$CONSOLE_ONLY" -eq 0 ]]; then
        info "Installing GTK2 IDE dependencies..."
        if sudo apt-get install -y --no-install-recommends "${GTK_DEPS[@]}" 2>/dev/null; then
            ok "GTK2 dependencies installed."
        else
            warn "GTK2 packages not available — falling back to console-only build."
            CONSOLE_ONLY=1
        fi
    fi
else
    info "Skipping dependency installation (--no-install-deps)."
fi

require_cmd cmake
require_cmd make

# ── detect GTK2 availability (if not already forced console-only) ─────────────
if [[ "$CONSOLE_ONLY" -eq 0 ]]; then
    if ! pkg-config --exists gtk+-2.0 2>/dev/null; then
        warn "gtk+-2.0 not found via pkg-config — building console-only."
        CONSOLE_ONLY=1
    fi
fi

[[ "$CONSOLE_ONLY" -eq 1 ]] && info "Build mode: console-only" \
                              || info "Build mode: console + GTK2 IDE"

# ── cmake configuration ───────────────────────────────────────────────────────
info "Configuring build in: ${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"

CMAKE_ARGS=(
    -S "${SCRIPT_DIR}"
    -B "${BUILD_DIR}"
    -DCMAKE_BUILD_TYPE=Release
    "-DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX}"
)

if [[ "$CONSOLE_ONLY" -eq 1 ]]; then
    CMAKE_ARGS+=(-D_nubasic_console=TRUE -DWITH_IDE=OFF)
else
    CMAKE_ARGS+=(-DWITH_IDE=ON)
fi

# Prefer Ninja when available
if command -v ninja >/dev/null 2>&1; then
    CMAKE_ARGS+=(-GNinja)
    BUILD_CMD=(ninja -j"${JOBS}")
else
    BUILD_CMD=(make -j"${JOBS}")
fi

cmake "${CMAKE_ARGS[@]}"

# ── build ─────────────────────────────────────────────────────────────────────
info "Building with ${JOBS} parallel jobs..."
cmake --build "${BUILD_DIR}" --config Release --parallel "${JOBS}"
ok "Build complete."

# ── run cpack to produce .deb ─────────────────────────────────────────────────
info "Generating Debian package with CPack..."
(
    cd "${BUILD_DIR}"
    cpack \
        -G DEB \
        -D CPACK_PACKAGING_INSTALL_PREFIX="${INSTALL_PREFIX}" \
        -D CPACK_SET_DESTDIR=ON \
        --config CPackConfig.cmake
)

# ── locate generated package ──────────────────────────────────────────────────
DEB_FILE="$(find "${BUILD_DIR}" -maxdepth 1 -name "*.deb" | sort | tail -1)"

if [[ -z "${DEB_FILE}" ]]; then
    die "No .deb file found in ${BUILD_DIR} — CPack may have failed."
fi

ok "Package created: ${DEB_FILE}"
echo ""
echo "  To inspect:  dpkg-deb -I '${DEB_FILE}'"
echo "  To install:  sudo dpkg -i '${DEB_FILE}'"
echo "               sudo apt-get install -f   # fix any missing deps"
echo ""

