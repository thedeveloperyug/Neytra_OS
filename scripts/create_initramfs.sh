#!/bin/bash
#
# create_initramfs.sh — repackage the CURRENT rootfs/ tree into
# output/initramfs.cpio.gz, without touching rootfs/ contents.
#
# Use this for a fast rebuild-and-boot loop after hand-editing files under
# rootfs/. Use build_rootfs.sh instead when the skeleton/BusyBox symlinks
# also need to be (re)created.

set -e

cd "$(dirname "$0")/.."

mkdir -p output
cd rootfs
find . | cpio -H newc -o 2>/dev/null | gzip > ../output/initramfs.cpio.gz
cd - >/dev/null

echo "initramfs recreated: output/initramfs.cpio.gz"

