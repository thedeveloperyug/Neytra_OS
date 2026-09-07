# Networking

> **Status: PLANNED — not implemented.** Every file described below is currently a one-line placeholder (e.g. `// NetworkManager.cpp - placeholder`). This doc describes the architecture implied by the existing folder/class scaffolding under [system/network/](../system/network/), as a design reference for implementing it — not a description of working code.

## Why networking is deferred

The root [README.md](../README.md)'s "Development Strategy" explicitly says *not* to start with networking — it's Phase 8+ in [roadmap.md](roadmap.md), after the kernel/rootfs/init/shell/QEMU loop (done) and the core C++ `system/init` + `system/shell` layers (in progress). See [architecture.md](architecture.md) for the full layering.

## Scaffolded classes

| Class | File | Inferred responsibility |
|---|---|---|
| `NetworkManager` | [system/network/NetworkManager.cpp](../system/network/NetworkManager.cpp) | Top-level coordinator: brings interfaces up/down, owns the other three classes |
| `DHCPClient` | [system/network/DHCPClient.cpp](../system/network/DHCPClient.cpp) | DHCP lease acquisition/renewal for an interface |
| `WifiManager` | [system/network/WifiManager.cpp](../system/network/WifiManager.cpp) | Wi-Fi scanning/association (relevant mainly for the Raspberry Pi target) |
| `SocketManager` | [system/network/SocketManager.cpp](../system/network/SocketManager.cpp) | Socket lifecycle helper, likely underpinning both `NetworkManager` and higher layers like `system/package/Downloader` |

## Proposed relationship to the rest of the system

Per the [subsystem map](diagrams/subsystem-map.svg), `NetworkManager` would be started by `system/init/ServiceManager` alongside the other subsystems, and would need `rootfs/etc/network/` (currently empty — see [rootfs.md](rootfs.md)) as its configuration source, plus `configs/network/` (also currently empty — see [project-structure.md](project-structure.md)) for versioned defaults.

## Design considerations for implementation (not yet decided)

- BusyBox already provides basic networking applets (`ifconfig`, `route`, etc. — check `rootfs/bin/busybox --list` on a built system for what's compiled in) which could be a stopgap before `NetworkManager` exists.
- `DHCPClient` will need raw socket or `netlink` access — this has security implications; see the planned `system/security/Sandbox` and `PermissionManager` for how privileged operations might eventually be gated.
- Nothing in `rootfs/etc/network/` or `configs/network/` defines a schema yet — that's a prerequisite design decision before writing `NetworkManager`.

## See also

- [architecture.md](architecture.md) — full layered architecture and status legend.
- [roadmap.md](roadmap.md) — where networking sits in the overall phase plan.
