#!/bin/bash
#
# run.sh — one-command entry point: build the rootfs/initramfs and boot
# Neytra OS in QEMU (x86_64). See docs/development-workflow.md for the
# full manual workflow and docs/qemu-setup.md for what run_qemu_x86.sh does.

set -e

# Step 0: always operate from the repo root, regardless of caller's cwd.
cd "$(dirname "$0")"

# Step 1: make sure the host has the tools this script depends on.
for cmd in qemu-system-x86_64 cpio gzip; do
    command -v "$cmd" >/dev/null 2>&1 || {
        echo "Missing required tool: $cmd (see dev_setup.md / dev_setup_env.sh)"
        exit 1
    }
done

# Step 2: the kernel build is slow and not something to trigger silently —
# just check it has been built already (see docs/kernel-build.md).
KERNEL_IMAGE="kernel/linux/arch/x86/boot/bzImage"
if [ ! -f "$KERNEL_IMAGE" ]; then
    echo "Kernel image not found: $KERNEL_IMAGE"
    echo "Build it first (see docs/kernel-build.md):"
    echo "  git clone https://github.com/torvalds/linux.git kernel/linux"
    echo "  cd kernel/linux && make defconfig && make -j\$(nproc)"
    exit 1
fi

# Step 3: build the rootfs skeleton + BusyBox symlinks and package the initramfs.
./scripts/build_rootfs.sh

# Step 4: boot the kernel + initramfs in QEMU (Ctrl+A then X to exit).
./scripts/run_qemu_x86.sh
