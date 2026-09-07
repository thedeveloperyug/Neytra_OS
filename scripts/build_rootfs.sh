#!/bin/bash
#
# build_rootfs.sh — prepare rootfs/ skeleton and package it into an initramfs.
#
# Idempotent: safe to re-run any time the rootfs/ tree changes.

set -e

cd "$(dirname "$0")/.."

echo "==> Ensuring rootfs skeleton directories exist"
mkdir -p rootfs/{bin,sbin,etc/init.d,etc/network,proc,sys,dev,tmp,home/root,mnt,media,opt,srv,var/log,var/run,usr/bin,usr/sbin,usr/lib,usr/include,lib}

if [ ! -x rootfs/bin/busybox ]; then
    echo "!! rootfs/bin/busybox is missing or not executable."
    echo "   Copy a static busybox binary to rootfs/bin/busybox before continuing."
    exit 1
fi

echo "==> Linking BusyBox applets"
cd rootfs/bin
for cmd in sh ls cat echo mount uname dmesg ps mkdir rm; do
    ln -sf busybox "$cmd"
done
cd - >/dev/null

echo "==> Fixing permissions"
chmod +x rootfs/init
chmod +x rootfs/sbin/shutdown 2>/dev/null || true

echo "==> Packaging rootfs into initramfs"
mkdir -p output
cd rootfs
find . | cpio -H newc -o | gzip > ../output/initramfs.cpio.gz
cd - >/dev/null

echo "Rootfs built and packaged: output/initramfs.cpio.gz"
