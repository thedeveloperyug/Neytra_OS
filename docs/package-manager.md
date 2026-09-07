# Package Manager

> **Status: PLANNED — not implemented.** Every file described below is currently a one-line placeholder (e.g. `// PackageManager.cpp - placeholder`). This doc describes the architecture implied by the existing folder/class scaffolding under [system/package/](../system/package/), as a design reference — not a description of working code.

## Why this is deferred

Per the root [README.md](../README.md)'s development strategy, the package manager is explicitly a later phase (see [roadmap.md](roadmap.md)), after the core boot loop and `system/init`/`system/shell` are working. See [architecture.md](architecture.md) for the full layering and current status.

## Scaffolded classes

| Class | File | Inferred responsibility |
|---|---|---|
| `PackageManager` | [system/package/PackageManager.cpp](../system/package/PackageManager.cpp) | Top-level coordinator: install/remove/upgrade/query, owns the other three classes |
| `Repository` | [system/package/Repository.cpp](../system/package/Repository.cpp) | Package index/metadata — what's available, versions, dependencies |
| `Downloader` | [system/package/Downloader.cpp](../system/package/Downloader.cpp) | Fetches package archives, presumably over the (also planned) network stack — see [networking.md](networking.md) |
| `Installer` | [system/package/Installer.cpp](../system/package/Installer.cpp) | Unpacks and places files on disk, updates local package state |

## Proposed relationship to the rest of the system

Per the [subsystem map](diagrams/subsystem-map.svg), `PackageManager` would be started by `system/init/ServiceManager`. `Downloader` implies a dependency on `system/network/NetworkManager` (or at least `SocketManager`) — meaning package management is realistically a **later** milestone than networking, not an independent one.

## Open design questions (not yet decided)

- **Package format** — no format (tarball + manifest? something custom?) is defined anywhere in the repo yet.
- **Where packages install to** — `usr/`, `opt/`, and `srv/` all already exist as empty standard directories in [rootfs/](../rootfs/) (see [rootfs.md](rootfs.md)) and are natural candidates.
- **Trust/verification** — signing or checksum verification would presumably involve the planned `system/security/` classes (`PermissionManager`, `Sandbox`).
- **Third-party sources** — [`third_party/`](../third_party/) already has `busybox/`, `dropbear/`, `zlib/` placeholder folders, suggesting an intent to vendor some dependencies directly rather than only fetch at runtime.

## See also

- [architecture.md](architecture.md) — full layered architecture and status legend.
- [networking.md](networking.md) — the prerequisite subsystem for `Downloader`.
- [roadmap.md](roadmap.md) — where package management sits in the overall phase plan.
