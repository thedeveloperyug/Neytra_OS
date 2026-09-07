# Roadmap

This is the real, current-state checklist for Neytra OS, following the phase order the project set for itself in the root [README.md](../README.md) ("Do NOT initially start with Raspberry Pi, GUI, networking, package manager, custom filesystem"). Status legend matches the rest of the docs: **ACTIVE**, **PARTIAL**, **PLANNED**.

| Phase | Goal | Status | Notes |
|---|---|---|---|
| 1 | Environment setup | ✅ ACTIVE | [dev_setup.md](../dev_setup.md) + [`dev_setup_env.sh`](../dev_setup_env.sh) install and verify the full toolchain |
| 2 | Linux kernel | ✅ ACTIVE (x86_64) | Built: `vmlinux` + `bzImage`, Linux 7.1.0-rc3 — see [kernel-build.md](kernel-build.md) |
| 3 | Root filesystem | ✅ ACTIVE | BusyBox v1.36.1 + symlinked applets, standard dirs — see [rootfs.md](rootfs.md) |
| 4 | Initramfs | ✅ ACTIVE | `output/initramfs.cpio.gz` built via `scripts/build_rootfs.sh` / `create_initramfs.sh` |
| 5 | QEMU boot | ✅ ACTIVE | `scripts/run_qemu_x86.sh` boots to an interactive shell reproducibly — see [qemu-setup.md](qemu-setup.md) |
| 6 | Custom shell, logger, process manager (C++) | 🚧 IN PROGRESS | `system/logger/Logger` is implemented (`neytra_logger` target + `tests/logger/logger_test.cpp`); `system/shell/`, `system/process/` are still empty stubs — see [architecture.md](architecture.md) |
| 6b | Custom init (C++) | 🚧 IN PROGRESS | Interface + dependency-injection skeleton implemented (`IInitManager`/`InitManager`, `IMountManager`/`MountManager`, `IServiceManager`/`ServiceManager`, built as `neytra_init`, verified by `tests/init/init_test.cpp`); `mountAll()`/`startAll()` bodies are still stubs. `rootfs/init` (shell script) is what actually runs today — see [boot-process.md](boot-process.md) |
| 7 | Networking, SSH, package manager | ⏳ PLANNED | `system/network/`, `system/package/` are empty stubs; `services/ssh/`, `services/networking/`, `services/updater/` are empty folders — see [networking.md](networking.md), [package-manager.md](package-manager.md) |
| 8 | Framebuffer graphics, GUI | ⏳ PLANNED | `system/gui/{WindowManager,Desktop,Renderer,Framebuffer}` are empty stubs |
| 9 | Security (users, permissions, sandboxing) | ⏳ PLANNED | `system/security/{UserManager,PermissionManager,Sandbox}` are empty stubs |
| 10 | Applications | ⏳ PLANNED | `apps/{terminal,editor,calculator,monitor,settings}/` contain only `.gitkeep` |
| 11 | Raspberry Pi deployment, ARM64 support | 🚧 PARTIAL | Boot files present in `boot/` but arm64 kernel build, imaging, and flashing are not wired up — see [raspberrypi.md](raspberrypi.md) |

## What "done" looks like for Phase 6/6b (the current frontier)

The next concrete milestone is making the C++ layer real, starting with the smallest useful slice:

1. ~~Give `system/logger/Logger` an actual implementation~~ — **done**: leveled console+file logging, built as `neytra_logger`, verified by `tests/logger/logger_test.cpp` (`ctest`). Retrofitted behind an `ILogger` interface so callers depend on the interface, not the concrete singleton (see [architecture.md](architecture.md#design-principles)).
2. ~~Design `system/init/`'s interfaces and wire dependency injection~~ — **done**: `IMountManager`/`IServiceManager`/`IInitManager` + constructor-injected `InitManager`, composition root in `system/init/main.cpp`, verified by `tests/init/init_test.cpp`.
3. Implement `MountManager::mountAll()` and `ServiceManager::startAll()` for real, well enough to replace `rootfs/init`'s three `mount` calls and banner, and boot-test it in QEMU alongside the existing shell script (don't remove `rootfs/init`'s safety net until the replacement is proven).
4. Implement `system/shell/{Shell,CommandParser,BuiltinCommands}` (behind interfaces, constructor-injected, per [architecture.md](architecture.md#design-principles)) enough to replace the final `exec /bin/sh` with the native shell for a handful of builtins (`cd`, `echo`, `exit`).
5. Only then move to `ServiceManager`'s real service-starting logic + the Phase 7+ subsystems, since they depend on init/service infrastructure existing first.

See [diagrams/subsystem-map.svg](diagrams/subsystem-map.svg) for the full planned class map and [architecture.md](architecture.md) for how it all fits together.

## Known cleanup items (not blocking, but worth tracking)

- Most `scripts/*.sh` are still `echo "... placeholder"` stubs: `build_kernel.sh`, `build_system.sh`, `clean.sh`, `create_image.sh`, `flash_sd.sh`, `run_qemu_arm64.sh`, `setup_env.sh`. See [development-workflow.md](development-workflow.md#4-script-reference) for the full table of what each should eventually do.
- Root `CMakeLists.txt` now builds `system/logger/` and `system/init/` (see Phase 6/6b above); the rest of `system/` and the root `Makefile` are still placeholders — nothing else is wired up to compile yet.
- `configs/`, `toolchain/`, `third_party/`, `tests/`, `qemu/{disks,firmware,logs,snapshots}/` are all empty scaffolding (`.gitkeep` only), reserved for later phases.
