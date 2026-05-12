<h1>NEYTRA OS</h1>

## Author: Yogesh Pandey
<h5>Copyright (c) 2026 Neytra OS Contributors<h5>

# README.md for Neytra OS


# Neytra OS

Neytra OS is a custom Linux-based operating system project focused on:

- Linux internals
- kernel engineering
- QEMU virtualization
- custom init systems
- C++ systems programming
- Raspberry Pi support later

---

# Goal

Boot a custom Linux OS inside QEMU using:

- Linux kernel
- BusyBox
- custom root filesystem
- custom init process

---

# DEVELOPMENT STRATEGY

Do NOT initially start with:

- Raspberry Pi
- GUI
- networking
- package manager
- custom filesystem

Start with:

1. kernel
2. rootfs
3. init
4. shell
5. QEMU boot

This is the correct professional workflow.

---

# PHASE 1 — Environment Setup

## STEP 1 — Create Workspace

```bash
mkdir -p ~/neytra-os
cd ~/neytra-os
```

---

## STEP 2 — Verify Required Tools

Verify:

```bash
qemu-system-x86_64 --version
gcc --version
g++ --version
busybox
```

If all commands work:

- environment ready

---

## STEP 3 — Create Project Structure

Run:

```bash
mkdir -p \
boot \
kernel \
rootfs \
system/init \
system/shell \
system/logger \
scripts \
output \
docs \
tests \
qemu
```

---

## STEP 4 — Initialize Git Repository

```bash
git init
```

Create `.gitignore`:

```bash
nano .gitignore
```

Content:

```gitignore
output/
*.img
*.iso
*.o
*.a
*.so
*.log
```

---

# PHASE 2 — Linux Kernel

## STEP 5 — Download Linux Kernel

```bash
git clone https://github.com/torvalds/linux.git kernel/linux
```

This may take time.

---

## STEP 6 — Configure Kernel

Go inside:

```bash
cd kernel/linux
```

Load default x86 config:

```bash
make defconfig
```

---

## STEP 7 — Build Kernel

```bash
make -j$(nproc)
```

This builds:

- Linux kernel
- drivers
- modules

Output:

```text
arch/x86/boot/bzImage
```

---

# PHASE 3 — Root Filesystem

## STEP 8 — Create RootFS Structure

Go back:

```bash
cd ~/neytra-os
```

Create directories:

```bash
mkdir -p rootfs/{bin,sbin,etc,proc,sys,dev,tmp,usr/bin}
```

---

## STEP 9 — Add BusyBox

Copy BusyBox:

```bash
cp /bin/busybox rootfs/bin/
```

Create command links:

```bash
cd rootfs/bin

for cmd in sh ls cat echo mount uname dmesg ps mkdir rm; do
    ln -s busybox $cmd
done
```

---

## STEP 10 — Create Init Script

Go back:

```bash
cd ~/neytra-os
```

Create file:

```bash
nano rootfs/init
```

Content:

```sh
#!/bin/sh

mount -t proc proc /proc
mount -t sysfs sysfs /sys
mount -t devtmpfs devtmpfs /dev

echo ""
echo "================================="
echo " Welcome to Neytra OS"
echo "================================="
echo ""

exec /bin/sh
```

Make executable:

```bash
chmod +x rootfs/init
```

---

# PHASE 4 — Initramfs

## STEP 11 — Create Initramfs

```bash
cd rootfs

find . | cpio -H newc -o | gzip > ../output/initramfs.cpio.gz
```

This creates:

- temporary root filesystem image

---

# PHASE 5 — First Boot

## STEP 12 — Boot Neytra OS in QEMU

Go back:

```bash
cd ~/neytra-os
```

Run:

```bash
qemu-system-x86_64 \
-kernel kernel/linux/arch/x86/boot/bzImage \
-initrd output/initramfs.cpio.gz \
-nographic \
-append "console=ttyS0 init=/init"
```

---

# EXPECTED SUCCESS OUTPUT

You should see:

```text
=================================
 Welcome to Neytra OS
=================================

#
```

This means:

- kernel boot successful
- rootfs mounted
- init executed
- shell working

You officially built your first Linux OS environment.

---

# PHASE 6 — Create Build Automation

## STEP 13 — Create Build Script

File:

```bash
nano scripts/build_rootfs.sh
```

Content:

```bash
#!/bin/bash

set -e

cd rootfs

find . | cpio -H newc -o | gzip > ../output/initramfs.cpio.gz

echo "Initramfs created successfully"
```

Make executable:

```bash
chmod +x scripts/build_rootfs.sh
```

---

## STEP 14 — Create QEMU Launch Script

File:

```bash
nano scripts/run_qemu.sh
```

Content:

```bash
#!/bin/bash

qemu-system-x86_64 \
-kernel kernel/linux/arch/x86/boot/bzImage \
-initrd output/initramfs.cpio.gz \
-nographic \
-append "console=ttyS0 init=/init"
```

Make executable:

```bash
chmod +x scripts/run_qemu.sh
```

Now boot easily:

```bash
./scripts/run_qemu.sh
```

---


Focus on:

- boot process
- init systems
- shell
- services
- process management

That is how real Linux systems evolve.

---

# FUTURE ROADMAP

## Phase 1

- custom shell
- logger
- process manager

## Phase 2

- networking
- SSH
- package manager

## Phase 3

- framebuffer graphics
- GUI

## Phase 4

- Raspberry Pi deployment
- ARM64 support

---
