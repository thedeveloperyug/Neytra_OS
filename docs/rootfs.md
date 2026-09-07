# Root Filesystem

`rootfs/` is the tree that gets packaged into `output/initramfs.cpio.gz` and mounted by the kernel as `/`. It's a real, working minimal Linux userland built around BusyBox — not a placeholder. See [../docs/diagrams/directory-structure.svg](diagrams/directory-structure.svg) for how it fits alongside the rest of the repo.

## Layout

```
rootfs/
├── init                  # PID 1 entry point (see boot-process.md)
├── bin/                  # BusyBox + applet symlinks
├── sbin/                 # shutdown + halt/reboot/poweroff symlinks
├── etc/
│   ├── inittab           # not active yet (see boot-process.md)
│   ├── hostname           -> "neytra-os"
│   ├── motd                -> "Welcome to Neytra OS"
│   ├── init.d/            # empty, reserved for startup scripts
│   └── network/           # empty, reserved for network config
├── proc/ sys/ dev/ tmp/   # standard kernel-managed mount points (empty at rest)
├── usr/{bin,sbin,lib,include}/  # empty, reserved for future userland growth
├── home/root/, mnt/, media/, opt/, srv/, var/{log,run}/  # standard FHS dirs, empty
└── lib/                  # empty, reserved for shared libraries
```

## `rootfs/bin/` — BusyBox

```
$ file rootfs/bin/*
rootfs/bin/busybox: ELF 64-bit LSB executable, x86-64, statically linked, stripped
rootfs/bin/cat, dmesg, echo, ls, mkdir, mount, ps, rm, sh, uname: symbolic link to busybox
```

`busybox` is a single static binary (`BusyBox v1.36.1`, the Ubuntu-packaged build) — every other file in `bin/` is a symlink to it. BusyBox looks at `argv[0]` to decide which applet to run, so `rootfs/bin/ls` and `rootfs/bin/busybox ls` behave identically. This is the standard embedded-Linux trick for getting a full coreutils-like toolset from one ~1-2MB binary instead of dozens of separate ones.

## `rootfs/sbin/` — shutdown handling

```
$ file rootfs/sbin/*
rootfs/sbin/shutdown: POSIX shell script
rootfs/sbin/halt, poweroff, reboot: symbolic link to shutdown
```

[`shutdown`](../rootfs/sbin/shutdown) is a real script (not BusyBox) that parses `-h`/`-r`/`-p`/a numeric delay, syncs filesystems, then calls the matching BusyBox applet (`halt -f`, `reboot -f`, or `poweroff -f`). The three symlinks let you invoke it under any of its conventional names. [`scripts/setup_shutdown.sh`](../scripts/setup_shutdown.sh) is what wires up those symlinks and the executable bit.

## `rootfs/etc/`

| File | Purpose | Status |
|---|---|---|
| `hostname` | Sets the system hostname to `neytra-os` | used by userland tools if read |
| `motd` | "Welcome to Neytra OS" message-of-the-day | used by login-style tools if read |
| `inittab` | BusyBox-`init`-style service table (`sysinit` runs `rcS`, then `respawn`s a shell) | **not active** — see [boot-process.md](boot-process.md#why-etcinittab-isnt-actually-used-yet) |
| `init.d/` | Empty — reserved for the startup scripts `inittab`'s `rcS` line implies | scaffold |
| `network/` | Empty — reserved for interface configuration | scaffold |

## `rootfs/init`

The actual PID 1 program (a shell script). Fully covered in [boot-process.md](boot-process.md#3-init-stage--pid-1) — mounts `proc`/`sysfs`/`devtmpfs`, prints a banner, traps `SIGTERM`/`SIGPWR` for clean shutdown, then drops into a shell.

## How the tree gets built and packaged

Two scripts cooperate (see [development-workflow.md](development-workflow.md#4-script-reference) for the full table):

- [`scripts/build_rootfs.sh`](../scripts/build_rootfs.sh) — idempotently ensures the skeleton directories exist, verifies `rootfs/bin/busybox` is present, (re)creates the applet symlinks, fixes executable permissions on `init`/`shutdown`, then packages everything.
- [`scripts/create_initramfs.sh`](../scripts/create_initramfs.sh) — a lighter-weight "just repackage" script for when you've hand-edited files under `rootfs/` and want a fast rebuild-and-boot loop without re-running the skeleton/symlink setup.

Both ultimately do the same packaging step:

```bash
cd rootfs
find . | cpio -H newc -o | gzip > ../output/initramfs.cpio.gz
```

`cpio -H newc` produces the "new ASCII" cpio format the Linux kernel's initramfs unpacker expects; `gzip` is optional but conventional (the kernel auto-detects and decompresses it).

## Standard FHS directories

`proc/`, `sys/`, `dev/`, `tmp/`, `mnt/`, `media/`, `opt/`, `srv/`, `var/{log,run}/`, `usr/{bin,sbin,lib,include}/`, `home/root/`, and `lib/` are all present but empty at rest — `proc`, `sys`, and `dev` get populated by the kernel's mounts in `rootfs/init`; the rest are placeholders following the Filesystem Hierarchy Standard so that tools which expect them (e.g. anything doing `cd /tmp`) don't fail.
