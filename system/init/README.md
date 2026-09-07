# `system/init/` — Init

## Role

Planned native replacement for [`rootfs/init`](../../rootfs/init) as PID 1: mount
filesystems, then start and supervise services. Today, `rootfs/init` (a POSIX shell
script) does this job for real; this module is where that logic moves once implemented.

## Status

**🚧 Interface + dependency-injection skeleton implemented; core logic still stub.**
`IInitManager`/`IMountManager`/`IServiceManager` are real interfaces, `InitManager` takes
its two collaborators and an `ILogger` via constructor injection, and everything compiles,
links (`neytra_init`), and is exercised by
[`tests/init/init_test.cpp`](../../tests/init/init_test.cpp) — but `MountManager::mountAll()`
and `ServiceManager::startAll()` both just `return false;` today. Nothing here runs at
boot yet.

## Architecture

![InitManager detailed workflow](design/init-workflow.svg)

`InitManager` is the top-level orchestrator: it delegates filesystem mounting to
`MountManager` and service startup/supervision to `ServiceManager`. See
[design/init-workflow.svg](design/init-workflow.svg) for the full planned call sequence,
step by step, compared against what `rootfs/init` actually does today.

## Files

| File | Responsibility |
|---|---|
| `main.cpp` | Composition root: constructs `MountManager`/`ServiceManager`, injects them + `Logger::instance()` into `InitManager`, calls `run()` |
| `IInitManager.hpp` | Interface: `run()` |
| `InitManager.hpp` / `.cpp` | Top-level orchestrator; holds `IMountManager&`/`IServiceManager&`/`ILogger&` injected via its constructor (not owned/constructed internally) |
| `IMountManager.hpp` | Interface: `mountAll()` |
| `MountManager.hpp` / `.cpp` | Will mount `proc`/`sysfs`/`devtmpfs`; `mountAll()` currently just `return false;` |
| `IServiceManager.hpp` | Interface: `startAll()` |
| `ServiceManager.hpp` / `.cpp` | Will start/supervise services, eventually reading `etc/init.d/*`; `startAll()` currently just `return false;` |

## Technical notes

- **Dependency injection in practice**: `InitManager`'s constructor takes
  `IMountManager&`, `IServiceManager&`, and `ILogger&` — it never constructs its own
  collaborators. `system/init/main.cpp` (the composition root) is the only place that
  builds the concrete `MountManager`/`ServiceManager`/`Logger::instance()` and wires them
  together. See [docs/architecture.md](../../docs/architecture.md#design-principles) for
  why, and [tests/init/init_test.cpp](../../tests/init/init_test.cpp) for a second,
  independent composition (proving the wiring — not just `main.cpp` — actually works).
- **Today's real PID 1** is still [`rootfs/init`](../../rootfs/init) — see
  [docs/boot-process.md](../../docs/boot-process.md) for its exact behavior (mounts,
  banner, `trap "poweroff -f" SIGTERM SIGPWR`, then `/bin/sh`).
- Per [docs/roadmap.md](../../docs/roadmap.md), the next step is implementing
  `MountManager::mountAll()` and `ServiceManager::startAll()` for real and boot-testing
  in QEMU **alongside** `rootfs/init` (don't remove the shell script's safety net until
  the C++ replacement is proven).
- `InitManager::run()` already logs through the injected `ILogger` rather than raw
  `printf` — that part of the pattern is real today, not just planned.

## See also

- [docs/architecture.md](../../docs/architecture.md)
- [docs/boot-process.md](../../docs/boot-process.md)
- [docs/roadmap.md](../../docs/roadmap.md)
