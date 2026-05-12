#!/bin/bash

set -e
echo -e ""
echo "Note: This script will restart your system to apply KVM permissions. Please save any work before proceeding."
echo " "
echo "CTRL+C to cancel or wait for the script to continue."
echo -e ""
echo "========================================="
echo " Author: Yogesh Pandey | Copyright (c) 2026 Neytra OS Contributors"
echo "========================================="
echo "Script will start in 10 seconds..."

for i in {10..1}
do
    echo -e "\a"
    echo -ne "Starting in $i seconds...\r"
    sleep 1
done

echo "========================================="
echo " Updating Ubuntu"
echo "========================================="


# sudo apt update
# sudo apt upgrade -y
sleep 2
echo "========================================="
echo " Installing Development Packages"
echo "========================================="

sudo apt install -y \
build-essential make cmake ninja-build gcc g++ clang pkg-config \
bc bison flex libssl-dev libelf-dev libncurses-dev \
dwarves cpio rsync kmod \
gcc-aarch64-linux-gnu g++-aarch64-linux-gnu \
qemu-system qemu-system-x86 qemu-system-arm \
qemu-efi-aarch64 qemu-efi-arm \
qemu-utils ovmf \
busybox-static \
dosfstools e2fsprogs parted fdisk gdisk mtools \
grub-pc-bin grub-common xorriso u-boot-tools \
gdb gdb-multiarch strace ltrace valgrind htop tree \
net-tools iproute2 openssh-client curl wget \
minicom screen picocom \
git git-lfs \
vim nano tmux ripgrep unzip zip \
device-tree-compiler

echo "Development packages installed successfully."
sleep 2
echo "========================================="
echo " Installing KVM Acceleration"
echo "========================================="

sudo apt install -y \
qemu-kvm \
libvirt-daemon-system \
virt-manager

echo "KVM acceleration packages installed successfully."
sleep 2
echo "========================================="
echo " Adding User To KVM Groups"
echo "========================================="

sudo usermod -aG kvm $USER
sudo usermod -aG libvirt $USER

echo "User $USER added to kvm and libvirt groups."
sleep 2

echo "========================================="
echo " Verifying Installation"
echo "========================================="

qemu-system-x86_64 --version

aarch64-linux-gnu-g++ --version

busybox | head -n 1

echo "All tools verified successfully."
sleep 2
echo "========================================="
echo " Neytra OS Development Environment Ready"
echo "========================================="

echo "Rebooting your system for KVM permissions to apply."

echo "System will reboot in 10 seconds..."

for i in {10..1}
do
    echo -ne "Rebooting in $i seconds...\r"
    sleep 1
done

echo ""
sudo reboot