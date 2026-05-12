#!/bin/bash

set -e

cd rootfs

find . | cpio -H newc -o | gzip > ../output/initramfs.cpio.gz

echo "Initramfs created successfully"