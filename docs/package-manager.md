# Package Manager

> **Status: IMPLEMENTED (host-tested), not yet wired into boot or the shell.** Every
> class below is a real implementation — a real HTTP/1.1 downloader, a real tar-based
> installer, a real flat-file repository index — built as `neytra_package` and covered by
> [`tests/package/package_test.cpp`](../tests/package/package_test.cpp) (which spins up a
> throwaway local HTTP server to exercise the full pipeline end to end). See
> [system/package/README.md](../system/package/README.md) for the authoritative current
> status; this doc is the deeper design discussion.

## Why the package manager came after networking

Per the root [README.md](../README.md)'s development strategy, the package manager was a
later phase (see [roadmap.md](roadmap.md)) that depends on `system/network/` existing
first — `Downloader` is constructor-injected with `system/network/ISocketManager`. See
[architecture.md](architecture.md) for the full layering and current status.

## Classes

| Class | File | Real responsibility |
|---|---|---|
| `PackageManager` | [system/package/PackageManager.cpp](../system/package/PackageManager.cpp) | Top-level coordinator: `install()`/`remove()`/`isInstalled()`, orchestrates the other three via constructor-injected interfaces |
| `Repository` | [system/package/Repository.cpp](../system/package/Repository.cpp) | Loads a flat-file package index (`name\|version\|url` per line), `find()`/`list()` |
| `Downloader` | [system/package/Downloader.cpp](../system/package/Downloader.cpp) | Real HTTP/1.1 GET client over a raw TCP socket (via [`system/network/ISocketManager`](networking.md)) — no TLS/HTTPS |
| `Installer` | [system/package/Installer.cpp](../system/package/Installer.cpp) | Extracts archives by spawning the real `tar` binary via [`system/process/IProcessManager`](architecture.md), doesn't reimplement archive parsing |

## Relationship to the rest of the system

Per the [subsystem map](diagrams/subsystem-map.svg), `PackageManager` would eventually be
started by `system/init/ServiceManager` — that wiring doesn't exist yet. `Downloader`'s
dependency on `system/network/ISocketManager` is real and constructor-injected today, so
the "package management is a later milestone than networking" ordering from the original
design held up in practice.

## Known limitations / resolved and open questions

- **Package format**: resolved pragmatically — plain tar archives, extracted via the real `tar` binary (no custom format or manifest yet).
- **Where packages install to** — `PackageManager`'s constructor takes an `installRoot` string; `usr/`, `opt/`, `srv/` in [rootfs/](../rootfs/) remain natural candidates (see [rootfs.md](rootfs.md)), but nothing wires a default yet.
- **Trust/verification** — still open; no signing or checksum verification exists, and `system/security/` (also implemented now) isn't consulted before install.
- **Third-party sources** — [`third_party/`](../third_party/) still has only `busybox/`, `dropbear/`, `zlib/` placeholder folders; nothing is vendored through `PackageManager` yet.
- **No TLS** — `Downloader` only speaks plain `http://`, deliberately, to keep the first real implementation simple.

## See also

- [architecture.md](architecture.md) — full layered architecture and status legend.
- [networking.md](networking.md) — the prerequisite subsystem for `Downloader`, also now implemented.
- [roadmap.md](roadmap.md) — where package management sits in the overall phase plan.
