#!/bin/bash
#
# build_system.sh — build the system/ C++ layer and install its statically
# linked CLI tools into rootfs/, so they end up in the initramfs.

set -e

cd "$(dirname "$0")/.."

echo "==> Configuring CMake (build/)"
cmake -S . -B build

echo "==> Building system/ targets"
cmake --build build -j"$(nproc)"

echo "==> Installing neytra-log into rootfs/usr/bin/"
mkdir -p rootfs/usr/bin
cp build/neytra-log rootfs/usr/bin/neytra-log
chmod +x rootfs/usr/bin/neytra-log

echo "==> Installing neytra-shell into rootfs/usr/bin/"
cp build/neytra-shell rootfs/usr/bin/neytra-shell
chmod +x rootfs/usr/bin/neytra-shell

echo "==> Installing neytra-diag into rootfs/usr/bin/"
cp build/neytra-diag rootfs/usr/bin/neytra-diag
chmod +x rootfs/usr/bin/neytra-diag

echo "==> Installing neytra-netup into rootfs/usr/bin/"
cp build/neytra-netup rootfs/usr/bin/neytra-netup
chmod +x rootfs/usr/bin/neytra-netup

echo "system/ built and installed into rootfs/usr/bin/"

