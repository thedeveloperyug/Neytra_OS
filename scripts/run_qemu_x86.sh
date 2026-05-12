#!/bin/bash

qemu-system-x86_64 \
-kernel kernel/linux/arch/x86/boot/bzImage \
-initrd output/initramfs.cpio.gz \
-nographic \
-append "console=ttyS0 init=/init"