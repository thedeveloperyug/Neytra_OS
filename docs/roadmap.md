# Roadmap

This is the real, current-state checklist for Neytra OS, following the phase order the project set for itself in the root [README.md](../README.md) ("Do NOT initially start with Raspberry Pi, GUI, networking, package manager, custom filesystem"). Status legend matches the rest of the docs: **ACTIVE**, **PARTIAL**, **PLANNED**.

| Phase | Goal | Status | Notes |
|---|---|---|---|
| 1 | Environment setup | ✅ ACTIVE | [dev_setup.md](../dev_setup.md) + [`dev_setup_env.sh`](../dev_setup_env.sh) install and verify the full toolchain |
| 2 | Linux kernel | ✅ ACTIVE (x86_64) | Built: `vmlinux` + `bzImage`, Linux 7.1.0-rc3 — see [kernel-build.md](kernel-build.md) |
| 3 | Root filesystem | ✅ ACTIVE | BusyBox v1.36.1 + symlinked applets, standard dirs — see [rootfs.md](rootfs.md) |
| 4 | Initramfs | ✅ ACTIVE | `output/initramfs.cpio.gz` built via `scripts/build_rootfs.sh` / `create_initramfs.sh` |
| 5 | QEMU boot | ✅ ACTIVE | `scripts/run_qemu_x86.sh` boots to an interactive shell reproducibly — see [qemu-setup.md](qemu-setup.md) |
| 6 | Custom shell, logger, process manager (C++) | 🚧 IN PROGRESS | `system/logger/Logger` is implemented and **actually running at boot** — `rootfs/init` calls the statically-linked `neytra-log` CLI, which logs to console + `/var/log/neytra.log` inside the booted VM (verified end-to-end in QEMU). `system/shell/` (real REPL + builtins + external spawn) and `system/process/` (real fork/exec/wait, nice-value scheduling, FIFO IPC) are **implemented, host-tested, and now boot-reachable too** — `scripts/build_system.sh` installs a statically-linked `neytra-shell` CLI into `rootfs/usr/bin/`, runnable manually from the BusyBox prompt after boot (verified end-to-end in QEMU: real builtins, real external command spawning via `ProcessManager`). It is not the automatic login shell yet — BusyBox still runs at boot and hands off to `/bin/sh` — see [system/README.md](../system/README.md) for the full testing guide |
| 6b | Custom init (C++) | 🚧 IN PROGRESS | `IInitManager`/`InitManager`, `IMountManager`/`MountManager`, `IServiceManager`/`ServiceManager` are implemented for real — `MountManager::mountAll()` issues real `mount(2)` calls, `ServiceManager::startAll()` parses a config and spawns services via `system/process/ProcessManager` — built as `neytra_init`, verified by `tests/init/init_test.cpp` (against a safe temp tmpfs, not real `/proc`/`/sys`/`/dev`). `rootfs/init` (shell script) is what actually runs at boot today — see [boot-process.md](boot-process.md) |
| 7 | Networking, SSH, package manager | 🚧 PARTIAL | `system/network/` (real sockets, interface enumeration, a full RFC 2131 DHCP client, sysfs wifi-interface detection) and `system/package/` (real HTTP/1.1 downloader, tar-based installer, repository index, all wired together) are **implemented and host-tested** (`tests/network/network_test.cpp`, `tests/package/package_test.cpp`) but not invoked from boot or the shell yet; `services/ssh/`, `services/networking/`, `services/updater/` are still empty folders — see [networking.md](networking.md), [package-manager.md](package-manager.md) |
| 8 | Framebuffer graphics, GUI | ⏳ PLANNED | `system/gui/{WindowManager,Desktop,Renderer,Framebuffer}` are empty stubs — deliberately untouched; explicitly deferred to last |
| 9 | Security (users, permissions, sandboxing) | 🚧 PARTIAL | `system/security/{UserManager,PermissionManager,Sandbox}` are **implemented and host-tested** (`tests/security/security_test.cpp`) — real passwd-style user table, a per-uid/resource permission rule table, and a real fork+chroot+rlimit sandbox — but no other module calls into it yet (no enforcement wired up) |
| 10 | Applications | ⏳ PLANNED | `apps/{terminal,editor,calculator,monitor,settings}/` contain only `.gitkeep` |
| 11 | Raspberry Pi deployment, ARM64 support | 🚧 PARTIAL | Boot files present in `boot/` but arm64 kernel build, imaging, and flashing are not wired up — see [raspberrypi.md](raspberrypi.md) |

## What "done" looks like for Phase 6/6b/7/9 (the current frontier)

The C++ layer went from one working module (`Logger`) to eight in one push — all of `system/` except `system/gui/` now has a real, host-tested implementation behind interfaces:

1. ~~Give `system/logger/Logger` an actual implementation~~ — **done**: leveled console+file logging, built as `neytra_logger`, verified by `tests/logger/logger_test.cpp` (`ctest`), and **actually running at boot**.
2. ~~Design `system/init/`'s interfaces and wire dependency injection~~ — **done**: `IMountManager`/`IServiceManager`/`IInitManager` + constructor-injected `InitManager`.
3. ~~Implement `MountManager::mountAll()` and `ServiceManager::startAll()` for real~~ — **done**: real `mount(2)` calls and real service-spawning via `IProcessManager`. **Not yet boot-tested in QEMU** — that's the next concrete step for init specifically (don't remove `rootfs/init`'s safety net until it is).
4. ~~Implement `system/shell/{Shell,CommandParser,BuiltinCommands}`~~ — **done**: full REPL, quote-aware parsing, 8 builtins, external command spawning. **Not yet wired in** to replace `exec /bin/sh` in `rootfs/init`.
5. ~~Implement `system/process/{ProcessManager,Scheduler,IPC}`~~ — **done**: real fork/exec/wait, nice-value scheduling, FIFO-based IPC. Already consumed by `system/init/ServiceManager`.
6. ~~Implement `system/network/`, `system/package/`, `system/security/`, `system/drivers/`~~ — **done**: see the Phase 7/9 notes above and each module's own README for exactly what's real vs. deliberately out of scope (e.g. `WifiManager::scan()`, GPIO/I2C/SPI needing real Raspberry Pi hardware).

Next concrete steps, roughly in order:

1. Boot-test `system/init/` in QEMU (behind a flag/alongside `rootfs/init`), since host tests alone don't prove a real boot.
2. Wire `system/shell/Shell` in as an optional login shell (still keeping BusyBox as the fallback) and give it a `pkg` builtin that calls into `system/package/PackageManager`.
3. Have `system/process/ProcessManager` (or whoever spawns untrusted/privileged work) actually call `system/security/PermissionManager::check()` before proceeding — right now security exists but nothing consults it.
4. Only after that: Phase 8 (GUI), deliberately last per the project's own stated philosophy.

See [diagrams/subsystem-map.svg](diagrams/subsystem-map.svg) for the (now dated — see the architecture doc's note) planned class map and [architecture.md](architecture.md) for how it all fits together.

## Known cleanup items (not blocking, but worth tracking)

- Most `scripts/*.sh` are still `echo "... placeholder"` stubs: `build_kernel.sh`, `clean.sh`, `create_image.sh`, `flash_sd.sh`, `run_qemu_arm64.sh`, `setup_env.sh`. `build_system.sh` and `build_rootfs.sh` are real. See [development-workflow.md](development-workflow.md#4-script-reference) for the full table of what each should eventually do.
- Root `CMakeLists.txt` now builds all of `system/` except `system/gui/` (8 static/executable targets — see [architecture.md](architecture.md)); the root `Makefile` is still a placeholder.
- `configs/`, `toolchain/`, `third_party/`, `qemu/{disks,firmware,logs,snapshots}/` are all empty scaffolding (`.gitkeep` only), reserved for later phases. `tests/` now has 8 real suites (one per implemented module), all passing under `ctest`.
