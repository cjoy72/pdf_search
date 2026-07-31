#!/bin/bash

# Exit script immediately on unhandled command failures where appropriate
set -e

echo "========================================="
echo "       PDF Search Installer Script       "
echo "========================================="

# 1. Check Operating System
echo "[1/3] Checking Operating System..."
OS="$(uname -s)"
IS_SUPPORTED=false

if [ "$OS" = "Darwin" ]; then
    echo "      Detected: macOS (Supported)"
    IS_SUPPORTED=true
elif [ "$OS" = "Linux" ]; then
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        echo "      Detected: Linux (${NAME:-Linux}) (Supported)"
    else
        echo "      Detected: Linux (Supported)"
    fi
    IS_SUPPORTED=true
fi

if [ "$IS_SUPPORTED" = false ]; then
    echo "ERROR: Unsupported Operating System ($OS)."
    echo "This script only supports macOS and Linux distributions."
    exit 1
fi

REPO_URL="https://github.com/cjoy72/pdf_search.git"
GIT_DIR="pdf_search"
BUILD_SUCCESS=false

# Setup cleanup function to delete git folder upon exit
cleanup() {
    echo ""
    echo "[3/3] Cleaning up git folder..."
    if [ -d "$GIT_DIR" ]; then
        rm -rf "$GIT_DIR"
        echo "      Deleted '$GIT_DIR' directory."
    fi

    if [ "$BUILD_SUCCESS" = true ]; then
        echo ""
        echo "========================================="
        echo " SUCCESS: PDF Search installed successfully!"
        echo "========================================="
    else
        echo ""
        echo "========================================="
        echo " FAILED: Installation/Compilation failed."
        echo "========================================="
    fi
}

trap cleanup EXIT

# 2. Clone repository and run make
echo ""
echo "[2/3] Cloning repository and building..."

# Remove any existing folder with the same name before cloning
if [ -d "$GIT_DIR" ]; then
    rm -rf "$GIT_DIR"
fi

echo "      Cloning $REPO_URL..."
git clone "$REPO_URL" "$GIT_DIR"

cd "$GIT_DIR"

echo "      Running 'make'..."
# Temporarily disable 'set -e' to gracefully capture 'make' failure
set +e
make
MAKE_EXIT_CODE=$?
set -e

cd ..

if [ $MAKE_EXIT_CODE -eq 0 ]; then
    BUILD_SUCCESS=true
else
    BUILD_SUCCESS=false
    exit 1
fi
