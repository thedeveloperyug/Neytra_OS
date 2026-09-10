# Architecture

Neytra OS is built as a stack of layers, the same way most Linux-based systems are: hardware, firmware, kernel, root filesystem, init, shell, then everything else (services, GUI, apps). What makes this doc useful is that it tells you **which of those layers are real today** and which are scaffolding for later phases — see [../README.md](../README.md) for the project's own "don't start with the GUI" philosophy that explains why the GUI, `apps/`, and `services/` are still stubs while the rest of `system/` is not.

## Layered view

![Neytra OS layered system architecture](diagrams/system-architecture.svg)

Reading it bottom-up:

1. **Hardware** — an x86_64 QEMU virtual machine today; a Raspberry Pi 4B (Broadcom BCM2711, ARM64) is the future target.
2. **Firmware / bootloader** — QEMU's direct kernel boot (`-kernel`/`-initrd`, no bootloader needed) today; the RPi path (GPU firmware reading `config.txt`/`cmdline.txt`) has its boot files in place but is untested end-to-end.
3. **Linux kernel** — a real, unmodified upstream kernel (`kernel/linux`), configured and built for x86_64. See [kernel-build.md](kernel-build.md).
4. **Initramfs / root filesystem** — `rootfs/`, populated with BusyBox and packaged into `output/initramfs.cpio.gz`. See [rootfs.md](rootfs.md).
5. **Init process (PID 1)** — `rootfs/init`, a POSIX shell script. This is what the kernel actually executes today. See [boot-process.md](boot-process.md).
6. **Shell** — BusyBox's `/bin/sh`, reached via `exec /bin/sh` at the end of `rootfs/init`.
7. **System services, applications** — `system/` is now real and host-tested for everything except the GUI (see the module map below); `apps/`, `services/` are still empty scaffolding. None of `system/`'s new code runs at boot yet except `Logger` — it's built and tested on the host, not yet invoked from `rootfs/init`.

## Why a C++ `system/` layer exists but barely runs yet

The repository has the *shape* of a self-hosted C++ system layer (an init manager, a shell, drivers, a GUI toolkit, networking, a package manager, process management, security) under [system/](../system/). As of this writing, every module except `system/gui/` has a real implementation behind interfaces, built as a CMake static library, and covered by its own `ctest` suite — for example:

```cpp
// system/process/ProcessManager.cpp (excerpt) - real, not a placeholder
ProcessId ProcessManager::spawn(const std::string& command, const std::vector<std::string>& args) {
    pid_t pid = fork();
    if (pid == 0) { /* ...execvp(...)... */ }
    // ...waitpid-based bookkeeping in ProcessInfo...
}
```

The root [README.md](../README.md) explicitly lays out a "professional workflow" that says *not* to start with the GUI, and to nail the kernel → rootfs → init → shell → QEMU boot loop first. That loop is done (see the diagram above — everything in the bottom half is solid blue). The `system/` C++ layer came next, module by module, starting with `system/logger/Logger` (the template: fully working *and* wired into the real boot path) and then all of `system/init/`, `system/shell/`, `system/process/`, `system/network/`, `system/package/`, `system/security/`, `system/drivers/` in one pass (see [Design principles](#design-principles) below for the pattern every one of them follows).

**"Implemented" here means real syscalls/protocols behind a passing host test — not necessarily wired into the boot path.** Only `Logger` is both. The rest compile, link, and are exercised by real fork/exec, real sockets, a real DHCP handshake, a real HTTP GET, a real tar extraction, and so on, but nothing outside their own tests calls into most of them yet (see each module's README "Technical notes" for exactly what is and isn't connected).

### `system/` module map

![Planned system subsystem map](diagrams/subsystem-map.svg)

> The diagram predates this implementation pass and still shows everything as planned stubs — the table below is the up-to-date source of truth. Each module also has its own `design/*-workflow.svg` (linked from its README) showing its real internal call sequence.

| Folder | Classes | Role / status |
|---|---|---|
| `system/init/` | `IInitManager`/`InitManager`, `IMountManager`/`MountManager`, `IServiceManager`/`ServiceManager` | Replace `rootfs/init` as PID 1 — **✅ implemented**, real `mount(2)`/service-spawning, built as `neytra_init`, covered by `tests/init/init_test.cpp`; not yet invoked at boot |
| `system/logger/` | `Logger` | Central leveled logger (console + optional file sink) — **✅ implemented and running at boot**, built as `neytra_logger`, covered by `tests/logger/logger_test.cpp` |
| `system/shell/` | `Shell`, `CommandParser`, `BuiltinCommands` | Replace BusyBox `/bin/sh` with a native Neytra shell — **✅ implemented**, real REPL + 8 builtins + external spawn, built as `neytra_shell`, covered by `tests/shell/shell_test.cpp`; not yet wired into boot |
| `system/process/` | `ProcessManager`, `Scheduler`, `IPC` | Process supervision and inter-process communication — **✅ implemented**, real fork/exec/wait, `setpriority`/`getpriority`, FIFO-based IPC, built as `neytra_process`, covered by `tests/process/process_test.cpp`; already consumed by `system/init/ServiceManager` |
| `system/network/` | `NetworkManager`, `DHCPClient`, `WifiManager`, `SocketManager` | Networking stack (see [networking.md](networking.md)) — **✅ implemented**, real sockets/interface enumeration/RFC 2131 DHCP client, built as `neytra_network`, covered by `tests/network/network_test.cpp`; `WifiManager::scan()` is an honest no-op (no wireless hardware to test against) |
| `system/package/` | `PackageManager`, `Downloader`, `Installer`, `Repository` | Package management (see [package-manager.md](package-manager.md)) — **✅ implemented**, real HTTP/1.1 downloader + tar-based installer, built as `neytra_package`, covered by `tests/package/package_test.cpp` (against an in-process fake HTTP server) |
| `system/security/` | `UserManager`, `PermissionManager`, `Sandbox` | Users, permissions, process sandboxing — **✅ implemented**, real passwd-style user table + rule-based permission checks + fork/chroot/rlimit sandbox, built as `neytra_security`, covered by `tests/security/security_test.cpp`; not yet consulted by any other module |
| `system/drivers/` | `GPIO`, `I2C`, `SPI`, `UART` | Raspberry Pi hardware access (see [raspberrypi.md](raspberrypi.md)) — **✅ implemented**, real sysfs GPIO / `/dev/i2c-*` / `/dev/spidev*` / termios UART, built as `neytra_drivers`, covered by `tests/drivers/drivers_test.cpp` (GPIO/I2C/SPI fail gracefully without real Pi hardware; UART is tested for real via a pty) |
| `system/gui/` | `WindowManager`, `Desktop`, `Renderer`, `Framebuffer` | Framebuffer-based GUI — *stub, deliberately deferred to last* |

The ownership arrows in the diagram (`InitManager` → `ServiceManager` → each subsystem) are a **proposed** hierarchy inferred from the folder layout, not something expressed in code yet — `ServiceManager` spawns configured services as opaque processes today, it doesn't call into their APIs directly. Treat the diagram as a starting design, not a constraint.

## Build wiring status

`CMakeLists.txt` builds two real targets so far: `neytra_logger` (static library from
`system/logger/Logger.cpp`) and `neytra_init` (static library from `system/init/*.cpp`,
linked against `neytra_logger`), each with a `ctest`-registered smoke test
(`logger_test`, `init_test`). The rest of `system/` still has no build target — each
subsystem needs its own `add_library`/`add_executable` added as it gains real logic,
following the pattern below.

## Design principles

This section exists because a from-scratch OS is exactly the kind of project where early
shortcuts (global singletons, managers that `new` their own dependencies, everything
including everything) calcify into a tightly-coupled mess by the time you have a dozen
subsystems. The rules below are mandatory for every `system/` module from here on —
`system/init/` is the reference implementation; read it alongside this section.

### 1. Depend on interfaces, not concrete classes (Dependency Inversion)

Every module that other code needs to call gets a pure-virtual interface (`IFoo`)
separate from its concrete implementation (`Foo : public IFoo`):

```cpp
class IMountManager {
public:
    virtual ~IMountManager() = default;
    virtual bool mountAll() = 0;
};

class MountManager : public IMountManager {
public:
    bool mountAll() override;
};
```

Callers hold a reference/pointer to `IMountManager`, never to `MountManager` directly.
This is what makes modules swappable: a test can hand `InitManager` a fake
`IMountManager` that always succeeds (or always fails), with zero changes to
`InitManager` itself.

### 2. Wire dependencies in through the constructor (Dependency Injection)

A module never constructs its own collaborators. `InitManager` doesn't create a
`MountManager` internally — it receives `IMountManager&`, `IServiceManager&`, and
`ILogger&` as constructor arguments:

```cpp
class InitManager : public IInitManager {
public:
    InitManager(IMountManager& mountManager, IServiceManager& serviceManager, ILogger& logger);
    bool run() override;
private:
    IMountManager& mountManager_;
    IServiceManager& serviceManager_;
    ILogger& logger_;
};
```

This keeps `InitManager` ignorant of *which* mount manager or logger it's talking to —
it only knows the interface's contract.

### 3. Construct and wire concrete types in one place (Composition Root)

Somewhere has to actually build the real objects and hand them to each other — that
place is `main.cpp` (or a test's `main()`), never buried inside business logic:

```cpp
// system/init/main.cpp
int main_init() {
    MountManager mountManager;
    ServiceManager serviceManager;
    InitManager initManager(mountManager, serviceManager, Logger::instance());
    return initManager.run() ? 0 : 1;
}
```

`tests/init/init_test.cpp` does the same wiring independently, which is exactly the
point: the composition root is the *only* code that needs to change to swap an
implementation, add a fake for testing, or reconfigure the system.

### 4. `Logger`/`ILogger` is the one deliberate exception, and only partially

Threading a logger reference through every constructor in the codebase is impractical —
logging is a cross-cutting concern almost everything needs. So `Logger::instance()`
(a singleton) exists as a convenience **default**, but the rule is: **only composition
roots (`main.cpp`, `tests/*/*_test.cpp`) are allowed to call `Logger::instance()`
directly.** Everything else — like `InitManager` above — takes `ILogger&` as a
constructor parameter and never knows it's talking to a singleton. This gets you the
convenience of a global logger without every module being compile-time coupled to the
concrete `Logger` class.

### 5. Patterns reserved for later (don't pre-build these speculatively)

These are already decided, but intentionally **not** implemented anywhere yet, because
building them before there's a second real use case is speculative complexity for no
current payoff ([YAGNI](https://en.wikipedia.org/wiki/You_aren%27t_gonna_need_it)).
**Singleton is not in this list and isn't a substitute for any of them** — see the
comparison below for why they solve different problems than `Logger`'s singleton does.

- **Factory Method** for centralizing "which concrete class to create" once that decision
  actually varies. Concrete trigger: `ServiceManager::startAll()` will need to turn each
  line of `etc/init.d/*` config into a `Process`/`Service` object — the exact type/setup
  varies per entry, so a `createServiceFrom(configLine)` factory keeps that branching out
  of `ServiceManager` itself. Build it *when* `ServiceManager` gets real, not before.
- **Abstract Factory** for producing a whole *family* of related objects that must stay
  mutually consistent. Concrete trigger: `system/drivers/` once Neytra OS targets a
  *second* real board — an `IPlatformDriverFactory` would produce the matching set of
  GPIO/I2C/SPI/UART drivers for whichever board you're actually on, so you can never
  accidentally wire one board's driver next to another's. Today there's only ever one
  target per build, so a plain `if`/`else` in the composition root is enough — don't
  build the factory until a second real platform exists.
- **Observer / event bus** for *cross-module* notifications (e.g. `ServiceManager`
  publishing "service crashed" for anything to subscribe to) instead of modules calling
  directly into unrelated modules. Reach for this once two independent subsystems need
  to react to the same event (Phase 7+ territory — see [roadmap.md](roadmap.md)).
- **Strategy pattern** for swappable algorithms — e.g. `Scheduler`'s ordering policy or
  `PermissionManager`'s policy evaluation — once there's a second real policy to swap
  between, not before.
- **PIMPL idiom** to hide a class's private members from its own header, if/when
  header-coupling starts causing painful full rebuilds in a much bigger `system/` tree.
  Not worth the extra indirection at the current scale (two modules).

### Pattern comparison at a glance

Singleton and Factory get confused for alternatives to each other surprisingly often —
they're not. Singleton answers *"how many instances exist, and how do I reach one?"*
Factory answers *"which concrete class do I even instantiate?"* A factory can (and often
does) hand out a singleton; they're orthogonal, not competing.

| Pattern | Problem it solves | Status here |
|---|---|---|
| Dependency Injection + Interfaces | Loose coupling; swap/fake any collaborator | ✅ the default everywhere |
| Composition Root | One place decides which concrete types get wired together | ✅ every `main.cpp` / `*_test.cpp` |
| Singleton | Exactly one instance, globally reachable | ✅ used once, for `Logger`, gated behind `ILogger` |
| Factory Method | Centralize "which concrete class" when creation varies | ⏳ planned for `ServiceManager` |
| Abstract Factory | Create a consistent *family* of related objects | ⏳ planned for `system/drivers/` (second board) |
| Strategy | Swap an algorithm at runtime behind a shared interface | ⏳ planned for `Scheduler`/`PermissionManager` |
| Observer / event bus | Decouple "something happened" from "who reacts" | ⏳ planned, Phase 7+ |
| Builder | Step-by-step construction of a complex object | not needed yet |
| PIMPL | Hide private members from a header to cut rebuild coupling | only if header coupling gets painful |

### 6. Don't apply this to everything reflexively

An interface with exactly one implementation forever (and no test that needs a fake)
is just ceremony. Plain data holders (structs, simple value types) don't need an `IFoo`
either. Use judgement — the goal is loose coupling where it matters (things that get
called across module boundaries, or that tests need to fake), not an `I`-prefixed
interface on every class in the codebase.

## See also

- [project-structure.md](project-structure.md) for a directory-by-directory reference.
- [roadmap.md](roadmap.md) for the phase checklist this architecture maps to.
- [development-workflow.md](development-workflow.md) for how to actually build and run what exists today.
