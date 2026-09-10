# Networking

> **Status: IMPLEMENTED and wired into boot.** Every class below is a real
> implementation — real BSD sockets, real interface enumeration, a real RFC 2131 DHCP
> client that actually applies the lease it gets (IP/netmask + default route via new
> `ioctl(SIOCSIFADDR/SIOCSIFNETMASK/SIOCADDRT)` calls), real sysfs Wi-Fi-interface
> detection — built as `neytra_network` and covered by
> [`tests/network/network_test.cpp`](../tests/network/network_test.cpp). `rootfs/init`
> runs a small CLI, `neytra-netup`, in the background at every boot to bring `lo` and any
> real interface up and DHCP it — verified end-to-end in QEMU (real lease, real
> `ifconfig`-visible IP, real kernel routing table entry, real `ping` to the gateway). See
> [system/network/README.md](../system/network/README.md) and
> [system/README.md](../system/README.md) (which has the full verified transcript) for
> current status; this doc is the deeper design discussion.

## Why networking came after the boot loop

The root [README.md](../README.md)'s "Development Strategy" explicitly says *not* to
start with networking until the kernel/rootfs/init/shell/QEMU loop is solid — see
[roadmap.md](roadmap.md) for where this landed once that was done. See
[architecture.md](architecture.md) for the full layering.

## Classes

| Class | File | Real responsibility |
|---|---|---|
| `NetworkManager` | [system/network/NetworkManager.cpp](../system/network/NetworkManager.cpp) | Real `getifaddrs()`-based interface enumeration; brings interfaces up/down via `ioctl(SIOCSIFFLAGS)`; assigns an IPv4 address/netmask via `ioctl(SIOCSIFADDR/SIOCSIFNETMASK)`; installs a default route via `ioctl(SIOCADDRT)` (all need `CAP_NET_ADMIN`) |
| `DHCPClient` | [system/network/DHCPClient.cpp](../system/network/DHCPClient.cpp) | Real DHCP lease acquisition: hand-built BOOTP/DHCP packets, full DISCOVER→OFFER→REQUEST→ACK exchange over raw UDP broadcast (needs root to bind port 68) |
| `WifiManager` | [system/network/WifiManager.cpp](../system/network/WifiManager.cpp) | Real sysfs-based wireless-interface detection; `scan()` is an honest no-op — full nl80211 support is out of scope without real wireless hardware to test against |
| `SocketManager` | [system/network/SocketManager.cpp](../system/network/SocketManager.cpp) | Real BSD socket wrapper (`socket`/`bind`/`connect`/`send`/`recv`), also used directly by [`system/package/Downloader`](../system/package/Downloader.cpp) |

## Relationship to the rest of the system

[`system/network/tools/neytra-netup.cpp`](../system/network/tools/neytra-netup.cpp) is the
real composition root: `rootfs/init` launches it in the background at every boot (see
[system/README.md](../system/README.md) for a full verified transcript). It brings `lo`
and any other real interface up, waits for carrier, DHCPs it, and applies the lease —
the first `system/network/` code to run automatically, not just via `neytra-diag`. It
doesn't go through `system/init/ServiceManager` (that still only spawns opaque commands
from a config file, not `NetworkManager` directly) — it's `rootfs/init` calling a small
statically-linked CLI, the same pattern as `neytra-log`. It would need
`rootfs/etc/network/` (currently empty — see [rootfs.md](rootfs.md)) as a real
configuration source (e.g. static IP instead of always-DHCP) plus `configs/network/`
(also currently empty — see [project-structure.md](project-structure.md)) for versioned
defaults — neither schema is defined yet, so DHCP-or-nothing is the only mode today.

## Known limitations (deliberate, not bugs)

- BusyBox already provides basic networking applets (`ifconfig`, `route`, etc. — check `rootfs/bin/busybox --list`, or just call e.g. `busybox ifconfig` directly even without a symlink) — useful for independently verifying what `neytra-netup` configured.
- `WifiManager::scan()` returns an empty list always — see [system/network/README.md](../system/network/README.md) for why.
- No caller currently consults `system/security/PermissionManager` before performing privileged network operations — the two modules exist independently today.
- Nothing in `rootfs/etc/network/` or `configs/network/` defines a schema yet.

## See also

- [architecture.md](architecture.md) — full layered architecture and status legend.
- [roadmap.md](roadmap.md) — where networking sits in the overall phase plan.
