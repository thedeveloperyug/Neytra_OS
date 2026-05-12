# Neytra OS Development Environment Setup

## Overview

This document contains:

1. Required Ubuntu packages
2. Development prerequisites

---

# Supported Host System

Recommended:

* Ubuntu 24.04 LTS
* Ubuntu 22.04 LTS

Architecture:

* x86_64

---

# Required Hardware

| Component      | Recommendation                        |
| -------------- | ------------------------------------- |
| Laptop/Desktop | Ubuntu system                         |
| RAM            | Minimum 8GB                           |
| Storage        | 30GB+ free space                      |
| CPU            | Intel/AMD with virtualization support |
| Raspberry Pi   | Optional for later deployment         |
| SD Card        | 32GB A2 U3 recommended                |

---

# Required Software Modules

## 1. Build Tools

These packages provide:

* GCC/G++ compilers
* build systems
* C/C++ development tools

```bash
build-essential
make
cmake
ninja-build
gcc
g++
clang
pkg-config
```

---

## 2. Linux Kernel Development Tools

These packages are required for:

* Linux kernel compilation
* menuconfig
* initramfs generation

```bash
bc
bison
flex
libssl-dev
libelf-dev
libncurses-dev
libncurses5-dev
dwarves
cpio
rsync
kmod
```

---

## 3. ARM64 Cross Compilation

Required for Raspberry Pi builds.

```bash
gcc-aarch64-linux-gnu
g++-aarch64-linux-gnu
```

---

## 4. QEMU Virtualization

Required for:

* x86_64 emulation
* ARM64 emulation
* kernel testing
* virtual machine booting

```bash
qemu-system
qemu-system-x86
qemu-system-arm
qemu-efi
qemu-utils
ovmf
```

---

## 5. BusyBox

Minimal Linux user-space utilities.

```bash
busybox-static
```

---

## 6. Filesystem Utilities

Required for:

* FAT32 creation
* EXT4 formatting
* SD card partitioning

```bash
dosfstools
e2fsprogs
parted
fdisk
gdisk
mtools
```

---

## 7. Bootloader Utilities

```bash
grub-pc-bin
grub-common
xorriso
u-boot-tools
```

---

## 8. Debugging Tools

Required for:

* debugging
* memory checking
* tracing

```bash
gdb
gdb-multiarch
strace
ltrace
valgrind
htop
tree
```

---

## 9. Networking Utilities

```bash
net-tools
iproute2
openssh-client
curl
wget
```

---

## 10. Serial Debugging Tools

Useful for Raspberry Pi debugging.

```bash
minicom
screen
picocom
```

---

## 11. Source Control

```bash
git
git-lfs
```

---

## 12. Developer Utilities

```bash
vim
nano
tmux
ripgrep
unzip
zip
```

---

## 13. Device Tree Tools

```bash
device-tree-compiler
```

---
