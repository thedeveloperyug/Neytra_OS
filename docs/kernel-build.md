# Kernel Build

Neytra OS uses a real, unmodified upstream Linux kernel — there's no fork or patch set, just standard configure-and-build. This doc covers the x86_64 path, which is built and working today; see [raspberrypi.md](raspberrypi.md) for the arm64 status.

## Current state (verified in this repo)

| Fact | Value |
|---|---|
| Source | `kernel/linux/` — a full `git clone` of [torvalds/linux](https://github.com/torvalds/linux) |
| Version | Linux **7.1.0-rc3** ("Baby Opossum Posse") |
| Configured arch | x86_64 (`kernel/linux/.config` has `CONFIG_X86_64=y`) |
| Built artifacts | `kernel/linux/vmlinux` (52MB), `kernel/linux/arch/x86/boot/bzImage`, `kernel/linux/arch/x86_64/boot/bzImage` |
| Tracked in git? | **No** — `kernel/` is in [`.gitignore`](../.gitignore); it's cloned locally per-developer, not committed |

## 1. Fetch the source

```bash
git clone https://github.com/torvalds/linux.git kernel/linux
```

This is a full kernel history clone, so it's large (the `kernel/` directory alone is ~9GB once built) and takes a while. It is deliberately not committed to the Neytra OS repo.

## 2. Configure

```bash
cd kernel/linux
make defconfig
```

`defconfig` produces a reasonable default `.config` for the detected/target architecture. The resulting `.config` in this repo confirms an x86_64 default configuration was used (look for the `# Linux/x86 7.1.0-rc3 Kernel Configuration` header comment at the top of the file).

To customize further, use `make menuconfig` (needs `libncurses-dev`, already listed in [dev_setup.md](../dev_setup.md)).

## 3. Build

```bash
make -j$(nproc)
```

This compiles the kernel image, all built-in drivers, and any modules enabled in `.config`. On success you get:

- `arch/x86/boot/bzImage` — the compressed, bootable kernel image QEMU/GRUB loads
- `vmlinux` — the uncompressed ELF kernel image, primarily useful for debugging with `gdb`/`crash` (matches your `.config` and toolchain, so symbols line up)

There is a `kernel/linux/arch/x86_64/boot/bzImage` too — same build, just also reachable through the arch-specific alias path.

> **Automation status:** [`scripts/build_kernel.sh`](../scripts/build_kernel.sh) is currently a placeholder (`echo "build_kernel.sh placeholder"`). Today, you run the two commands above by hand, per the root [README.md](../README.md) Phase 2 walkthrough. Wiring this script up to do exactly that (with a check for an existing `.config`) is good first-contribution material — see [roadmap.md](roadmap.md).

## Where this feeds into the boot pipeline

![Kernel track of the build pipeline](diagrams/build-pipeline.svg)

The kernel artifacts feed directly into [`scripts/run_qemu_x86.sh`](../scripts/run_qemu_x86.sh) via `-kernel`. See [qemu-setup.md](qemu-setup.md).

## Cross-compiling for ARM64 (Raspberry Pi)

Not done yet in this repo. The toolchain packages are already listed in [dev_setup.md](../dev_setup.md) (`gcc-aarch64-linux-gnu`, `g++-aarch64-linux-gnu`), and [`toolchain/cross/`](../toolchain/cross/) is reserved for this, but:

- there's no arm64 `.config` checked in or generated yet,
- `boot/kernel8.img` and `boot/bcm2711-rpi-4-b.dtb` already present in the repo were **not produced by a script in this repo** — treat them as external/prebuilt artifacts until a real arm64 build step exists.

To start that work, the general shape would be:

```bash
cd kernel/linux
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- bcm2711_defconfig   # or similar
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j$(nproc) Image dtbs
```

producing `arch/arm64/boot/Image` (or `Image.gz`) and `.dtb` files — this needs verification against whatever defconfig BCM2711 actually ships under `arch/arm64/configs/` in the cloned kernel tree before relying on it.

## See also

- [architecture.md](architecture.md) — where the kernel sits in the overall stack.
- [development-workflow.md](development-workflow.md) — the full build loop including rootfs and QEMU.
