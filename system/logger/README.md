# `system/logger/` — Logger

## Role

Centralized, leveled logging for every other `system/` subsystem. It's the first real
module in the C++ layer — everything built after it (init, shell, services, ...) is
expected to log through it instead of calling `printf`/`std::cout` directly.

## Status

**Implemented and running at real boot time**, not just host-tested. Built as the static
library `neytra_logger` (see root `CMakeLists.txt`), covered by
[`tests/logger/logger_test.cpp`](../../tests/logger/logger_test.cpp) — and as of
[`system/logger/tools/neytra-log.cpp`](tools/neytra-log.cpp), a statically-linked CLI
that [`rootfs/init`](../../rootfs/init) actually calls during boot (see
[docs/boot-process.md](../../docs/boot-process.md)), verified end-to-end in QEMU:
`/var/log/neytra.log` inside the booted VM contains real, timestamped boot events.

## Architecture

![Logger detailed workflow](design/logger-workflow.svg)

Every call goes through the single entry point `Logger::log()` (or its `debug`/`info`/
`warn`/`error` convenience wrappers), which fans out to up to two sinks: the console
(always) and a log file (only if `setLogFile()` was called first). Messages below the
configured minimum level are dropped before either sink is touched. See
[design/logger-workflow.svg](design/logger-workflow.svg) for the full call-by-call detail.

## Usage

From a shell script (e.g. `rootfs/init`) or any other process, via the CLI:

```sh
neytra-log <debug|info|warn|error> <tag> <message...>
```

- **Levels**, in increasing order: `debug` (dim, hidden by default) < `info` (cyan) <
  `warn` (yellow, `stderr`) < `error` (red, `stderr`).
- `<tag>` labels the source (e.g. `init`, `shell`); `<message...>` is every remaining
  argument joined with spaces — no quoting needed.
- Set `$NEYTRA_LOG_FILE` before calling it to also append a plain-text copy there.

From C++, through the interface (see [Technical notes](#technical-notes)):
`ILogger& logger = ...; logger.info("tag", "message");`

### What's logged today

Only [`rootfs/init`](../../rootfs/init) calls `neytra-log` so far, at three points during
boot (all `info`/`init`, and it sets `NEYTRA_LOG_FILE=/var/log/neytra.log` first):

```sh
neytra-log info init "mounted proc, sysfs, devtmpfs"
neytra-log info init "shutdown handler registered (SIGTERM/SIGPWR -> poweroff -f)"
neytra-log info init "boot complete, handing off to shell"
```

Every other implemented module (`system/init/`, `system/shell/`, `system/process/`,
`system/network/`, `system/package/`, `system/security/`, `system/drivers/`) takes an
`ILogger&` via constructor injection and logs through it already — but only `InitManager`'s
`main.cpp` path is actually invoked at boot today (the rest are exercised by their own
`ctest` suites, not by a running system). **Update this section** as each module gets
wired into the real boot/shell path, so it stays a true picture of what the OS actually
logs at runtime, not just what it's capable of.

## Files

| File | Responsibility |
|---|---|
| `ILogger.hpp` | `enum class LogLevel`; the `ILogger` interface (`log`/`debug`/`info`/`warn`/`error`) — depend on this, not `Logger`, from other modules |
| `Logger.hpp` | `Logger : public ILogger`, the default implementation, plus config (`setMinLevel`, `setLogFile`) and the `instance()` singleton accessor |
| `Logger.cpp` | Level filtering, ANSI coloring, `HH:MM:SS` timestamping, dual-sink fan-out |
| `tools/neytra-log.cpp` | Composition-root CLI: `neytra-log <level> <tag> <message...>`, reads `$NEYTRA_LOG_FILE` for the file sink. Built `-static` so it runs standalone in the minimal BusyBox rootfs (no shared libstdc++ needed). Installed into `rootfs/usr/bin/` by [`scripts/build_system.sh`](../../scripts/build_system.sh) |

## Technical notes

- **Depend on `ILogger`, not `Logger`.** Every other module should take `ILogger&` as a
  constructor parameter (see [system/init/InitManager](../init/README.md) for the
  reference example) rather than calling `Logger::instance()` itself. Only composition
  roots (`main.cpp`, `tests/*/*_test.cpp`) are allowed to touch the concrete singleton —
  see [docs/architecture.md](../../docs/architecture.md#design-principles).
- **Singleton access**: `Logger::instance()` returns a function-local static — no manual
  construction, no ownership to manage. It exists purely as the one pragmatic default
  everything is wired to at composition roots.
- **Levels**: `LogLevel::{Debug, Info, Warn, Error}`, in that increasing order (scoped
  enums compare by underlying value). Default minimum level is `Info`.
- **Console coloring** matches the palette used by the [`rootfs/init`](../../rootfs/init)
  boot banner: dim for Debug, bold cyan for Info, bold yellow for Warn, bold red for Error.
  `Warn`/`Error` are written to `stderr`; `Debug`/`Info` to `stdout`.
- **File sink** is plain text (no ANSI) and opened in append mode via `std::ofstream`;
  disabled until `setLogFile(path)` succeeds.
- **Not thread-safe** — there are no threads anywhere in this project yet, so no locking
  was added (see the repo's "don't add what isn't needed yet" convention). Revisit if/when
  `system/process/` starts spawning real concurrent work.
- **Getting from `system/` into the booted OS**: `neytra-log`'s `main()` is a composition
  root (it's allowed to call `Logger::instance()` directly, same as `system/init/main.cpp`)
  and is built fully static so it has no dynamic-linker dependency on a `libstdc++.so`
  that doesn't exist in the minimal rootfs. `rootfs/init` invokes it by absolute path
  (`/usr/bin/neytra-log`) with a fallback to a no-op if it's missing, so an initramfs built
  without it still boots.

## See also

- [docs/architecture.md](../../docs/architecture.md) — how this fits into the full stack, and the design-principles rules for depending on `ILogger`.
- [docs/roadmap.md](../../docs/roadmap.md) — Phase 6 status.
