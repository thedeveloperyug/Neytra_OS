# `system/process/` — Process

## Role

Process lifecycle management (spawn / monitor / reap) and inter-process communication
primitives, for use by `system/init/ServiceManager` and any future subsystem that needs
to run or talk to other processes.

## Status

**Implemented.** `ProcessManager` (real `fork`/`execvp`/`waitpid`), `Scheduler` (real
`setpriority`/`getpriority`), and `IPC` (real named-pipe/FIFO messaging via `mkfifo`) are
all real, built as `neytra_process`, covered by
[`tests/process/process_test.cpp`](../../tests/process/process_test.cpp).

## Architecture

![Process detailed workflow](design/process-workflow.svg)

`ProcessManager` spawns and reaps processes, consults `Scheduler` for run order, and uses
`IPC` for message passing between them. See
[design/process-workflow.svg](design/process-workflow.svg) for the full step-by-step detail.

## Files

| File | Responsibility (intended) |
|---|---|
| `ProcessManager.cpp` | Spawn, monitor, and reap child processes |
| `Scheduler.cpp` | Decide run order / priority among managed processes |
| `IPC.cpp` | Message-passing primitives between processes |

## Technical notes

- Follows the interface + constructor-injection pattern from
  [`system/init/`](../init/README.md) (`IProcessManager`/`IScheduler`/`IIPC` + concrete
  classes, `ILogger&` injected) — see
  [docs/architecture.md](../../docs/architecture.md#design-principles).
- `Scheduler` uses real `setpriority(2)`/`getpriority(2)` — raising your own niceness
  (deprioritizing) works unprivileged; lowering it (raising priority) needs `CAP_SYS_NICE`.
- `IPC` uses real `mkfifo(3)` named pipes; `send()`/`receive()` block for a reader/writer
  on the other end, matching real FIFO rendezvous semantics.
- Primary consumer today: `system/init/ServiceManager` (../init/README.md), which spawns
  configured services through this module's `ProcessManager`.
- Logs through [`system/logger/Logger`](../logger/README.md) via the injected `ILogger&`.
- Not on the critical path for the next roadmap milestone — see
  [docs/roadmap.md](../../docs/roadmap.md) (`init` and `shell` come first).

## See also

- [docs/architecture.md](../../docs/architecture.md)
- [docs/roadmap.md](../../docs/roadmap.md)
