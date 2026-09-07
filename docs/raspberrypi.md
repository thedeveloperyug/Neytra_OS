# Raspberry Pi

> **Status: PARTIAL.** Boot files are already present in the repo, but the arm64 kernel build, image creation, and SD flashing are not yet wired up or tested. Treat this as the "next hardware target" doc, not a "how to boot Neytra OS on your Pi today" guide.

## Target hardware

Per [dev_setup.md](../dev_setup.md): Raspberry Pi **4B**, Broadcom **BCM2711** SoC (quad-core Cortex-A72, ARM64/aarch64), 32GB+ A2/U3 SD card recommended.

## What's already in the repo

[`boot/`](../boot/) contains real Raspberry Pi boot-partition files:

| File | Purpose |
|---|---|
| `config.txt` | RPi firmware configuration (currently near-empty: just `adjust_overscan=0`) |
| `cmdline.txt` | Kernel command line — currently `root=/dev/mmcblk0p2 rootfstype=ext4 rw rootwait`, i.e. **boots from a real SD card ext4 partition**, not from an initramfs. This is a different boot model than the QEMU path (see note below). |
| `kernel8.img` | A prebuilt arm64 kernel image |
| `bcm2711-rpi-4-b.dtb` | Device tree blob describing the Pi 4B's hardware to the kernel |
| `initramfs.cpio.gz` | A copy of the same initramfs format used by the QEMU path |
| `overlays/` | Empty — reserved for device tree overlays (e.g. enabling specific peripherals) |

**Important:** `kernel8.img` and the `.dtb` were **not produced by any script in this repository** — there's no arm64 `.config` or cross-compile step checked in yet (see [kernel-build.md](kernel-build.md#cross-compiling-for-arm64-raspberry-pi)). Treat them as externally-sourced artifacts until a real build path exists, and don't assume they match whatever kernel version/config you'd get from building `kernel/linux` for arm64 yourself.

### A boot-model mismatch worth knowing about

`cmdline.txt`'s `root=/dev/mmcblk0p2 rootfstype=ext4 rw rootwait` tells the kernel to mount a real ext4 partition as root — but `initramfs.cpio.gz` is also present, which is the *other* model (RAM-based root, same as the QEMU path). These are two different strategies:

1. **Initramfs-based** (what QEMU does): kernel + initramfs, no real disk partition needed for root.
2. **Partition-based** (what `cmdline.txt` currently says): kernel boots, then mounts a real ext4 partition from the SD card as root — this needs an actual ext4 filesystem written to `/dev/mmcblk0p2` with a full rootfs on it (not just the compressed cpio archive).

Before attempting a real Pi boot, decide which model you want and make `cmdline.txt` consistent with it — right now it's set up for option 2, but the only rootfs artifact prepared by [`scripts/build_rootfs.sh`](../scripts/build_rootfs.sh) is the cpio archive for option 1.

## What's not done yet

- No `.config` for an arm64 kernel build (`configs/kernel/` is empty — see [project-structure.md](project-structure.md)).
- [`scripts/build_kernel.sh`](../scripts/build_kernel.sh), [`scripts/create_image.sh`](../scripts/create_image.sh), [`scripts/flash_sd.sh`](../scripts/flash_sd.sh), and [`scripts/run_qemu_arm64.sh`](../scripts/run_qemu_arm64.sh) are all still placeholders (`echo "... placeholder"`).
- [`system/drivers/`](../system/drivers/) (`GPIO`, `I2C`, `SPI`, `UART`) — the classes that would let Neytra OS talk to Pi-specific hardware — are empty stubs (see [architecture.md](architecture.md)).
- [`toolchain/cross/`](../toolchain/cross/), `toolchain/gcc/`, `toolchain/sysroot/` — reserved for the ARM64 cross-compiler, currently empty.

## Planned path to a real boot

![Alternate Raspberry Pi path in the build pipeline](diagrams/build-pipeline.svg)

1. Cross-compile a kernel for arm64 (`kernel-build.md`'s cross-compiling section).
2. Decide on initramfs-based vs. partition-based root (see mismatch note above) and make `cmdline.txt` match.
3. Implement `scripts/create_image.sh` to assemble a flashable disk image (boot partition with firmware/kernel/dtb/config, plus a root partition if going the partition-based route).
4. Implement `scripts/flash_sd.sh` to write that image to an SD card (this touches a physical block device — build in an explicit device confirmation prompt before it does anything destructive).
5. Boot the physical Pi 4B, confirm serial/HDMI console output.

## See also

- [kernel-build.md](kernel-build.md) — kernel build/cross-compile details.
- [roadmap.md](roadmap.md) — Raspberry Pi is Phase 11 in the overall plan.
- [dev_setup.md](../dev_setup.md) — required packages (`gcc-aarch64-linux-gnu`, `dosfstools`, `parted`, `u-boot-tools`, `device-tree-compiler`, etc.) are already listed there.
