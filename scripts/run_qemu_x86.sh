#!/bin/bash

# QEMU x86_64 emulation
# To exit: Press Ctrl+A then X (QEMU monitor escape)

qemu-system-x86_64 \
  -kernel kernel/linux/arch/x86/boot/bzImage \
  -initrd output/initramfs.cpio.gz \
  -nographic \
  -m 512M \
  -smp 2 \
  -enable-kvm \
  -no-reboot \
  -append "console=ttyS0 init=/init"