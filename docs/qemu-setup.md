# QEMU Setup

QEMU is how Neytra OS is tested today — no real hardware required. This doc explains the working invocation and its flags; see [boot-process.md](boot-process.md) for what happens after the VM starts.

## Verified host setup

```
QEMU emulator version 8.2.2 (Debian 1:8.2.2+ds-0ubuntu1.18)
```

Installed via the packages listed in [dev_setup.md](../dev_setup.md) (`qemu-system`, `qemu-system-x86`, `qemu-utils`, `ovmf`, plus `qemu-kvm`/`libvirt-daemon-system`/`virt-manager` for KVM acceleration, set up by [`dev_setup_env.sh`](../dev_setup_env.sh)).

## The working script

[`scripts/run_qemu_x86.sh`](../scripts/run_qemu_x86.sh):

```bash
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
```

## Flag-by-flag

| Flag | Meaning |
|---|---|
| `-kernel kernel/linux/arch/x86/boot/bzImage` | Boots this kernel image directly — no bootloader/GRUB needed, QEMU stands in for one |
| `-initrd output/initramfs.cpio.gz` | Loads the packaged rootfs into RAM as the initial root filesystem (see [rootfs.md](rootfs.md)) |
| `-nographic` | No QEMU GUI window; redirects the VM's serial console to your current terminal |
| `-m 512M` | 512MB of guest RAM |
| `-smp 2` | 2 virtual CPUs |
| `-enable-kvm` | Uses Linux's KVM for near-native CPU speed instead of full software emulation — requires `/dev/kvm` access (see below) |
| `-no-reboot` | If the guest requests a reboot, QEMU exits instead of restarting — makes crashes/panics visible instead of silently looping |
| `-append "console=ttyS0 init=/init"` | Kernel command line: `console=ttyS0` routes kernel/console output to the emulated serial port (which `-nographic` connects to your terminal); `init=/init` tells the kernel to run `rootfs/init` as PID 1 |

## Running it

```bash
./scripts/run_qemu_x86.sh
```

To exit: **Ctrl+A** then **X** (QEMU's monitor escape sequence for `-nographic` mode). Since `poweroff -f`/`reboot -f` are wired up inside the guest (see [rootfs.md](rootfs.md#rootfssbin--shutdown-handling)), you can also shut the VM down from inside the guest shell.

## KVM acceleration

`-enable-kvm` requires your user to be in the `kvm` group and `/dev/kvm` to exist (needs hardware virtualization support — Intel VT-x/AMD-V). [`dev_setup_env.sh`](../dev_setup_env.sh) handles installing `qemu-kvm`/`libvirt-daemon-system`/`virt-manager` and running `usermod -aG kvm $USER` (and reboots to apply group membership). If KVM isn't available (e.g. inside a VM without nested virtualization, or a CI runner), drop `-enable-kvm` — boot still works, just slower (full software emulation, TCG).

## Raspberry Pi / ARM64 in QEMU

[`scripts/run_qemu_arm64.sh`](../scripts/run_qemu_arm64.sh) is currently a placeholder. Note that QEMU's ARM machine support historically covers `raspi3b` well but has limited/no dedicated `raspi4b` machine type — so an arm64 QEMU path, if added, would likely be an approximation (e.g. `virt` machine with a generic arm64 kernel) rather than a true Pi 4 emulation. Real hardware (or at minimum `raspi3b` for a rough ARM64 sanity check) is the more faithful test target — see [raspberrypi.md](raspberrypi.md).

## See also

- [development-workflow.md](development-workflow.md) — the full build-then-boot loop this script sits at the end of.
- [diagrams/build-pipeline.svg](diagrams/build-pipeline.svg) — visual pipeline showing where this script fits.
