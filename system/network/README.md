# `system/network/` — Network

## Role

Networking stack: interface bring-up, DHCP, Wi-Fi, and sockets. See
[docs/networking.md](../../docs/networking.md) for the fuller design discussion this
README summarizes.

## Status

**Implemented and running at real boot time**, not just host-tested. `SocketManager`
(real BSD sockets), `NetworkManager` (real interface enumeration via `getifaddrs()`,
up/down via `ioctl(SIOCSIFFLAGS)`, address assignment via `ioctl(SIOCSIFADDR/SIOCSIFNETMASK)`,
default-route installation via `ioctl(SIOCADDRT)`), `DHCPClient` (a real RFC 2131
DISCOVER/OFFER/REQUEST/ACK client over raw UDP), and `WifiManager` (real sysfs
wireless-interface detection; `scan()` honestly returns empty since this project's QEMU
x86_64 target has no wireless hardware to scan with) are all real, built as
`neytra_network`, covered by [`tests/network/network_test.cpp`](../../tests/network/network_test.cpp).
As of [`system/network/tools/neytra-netup.cpp`](tools/neytra-netup.cpp), a statically-linked
CLI that [`rootfs/init`](../../rootfs/init) runs in the background at every boot, this
module brings `lo` and any real interface up, DHCPs it, and applies the lease for real —
verified end-to-end in QEMU (real lease, kernel-visible IP/route, real `ping` to the
gateway; see [system/README.md](../README.md) for the full transcript).

## Architecture

![Network detailed workflow](design/network-workflow.svg)

`NetworkManager` brings interfaces up and owns `DHCPClient`, `WifiManager`, and
`SocketManager`. See [design/network-workflow.svg](design/network-workflow.svg) for the
full step-by-step detail.

## Files

| File | Responsibility |
|---|---|
| `NetworkManager.cpp` | Interface enumeration/up-down, plus applying a lease: address, netmask, default route |
| `DHCPClient.cpp` | Real DHCP lease acquisition (DISCOVER/OFFER/REQUEST/ACK) for an interface |
| `WifiManager.cpp` | Wireless-interface detection; `scan()` is a documented no-op (see below) |
| `SocketManager.cpp` | Socket lifecycle helper underpinning the above and `system/package/Downloader` |
| `tools/neytra-netup.cpp` | Composition root built as the `neytra-netup` executable — brings interfaces up, DHCPs them, applies the lease; launched by `rootfs/init` at every boot |

## Technical notes

- Follows the interface + constructor-injection pattern from
  [`system/init/`](../init/README.md) (`ISocketManager`/`INetworkManager`/`IDHCPClient`/
  `IWifiManager` + concrete classes) — see
  [docs/architecture.md](../../docs/architecture.md#design-principles).
- `NetworkManager::setInterfaceUp()`/`setInterfaceAddress()`/`setDefaultGateway()` all
  need `CAP_NET_ADMIN` (root); each fails gracefully (logs a warning, returns false)
  without it rather than crashing. `neytra-netup` runs as root (PID 1's environment has
  no unprivileged user), so all three work for real at boot.
- `DHCPClient` speaks real RFC 2131 DISCOVER/OFFER/REQUEST/ACK over raw UDP — binding to
  port 68 also needs root. **Correction from an earlier assumption**: QEMU's *default*
  networking (no explicit `-netdev` needed) already attaches a NIC with built-in SLIRP
  DHCP server, so `acquire()` genuinely succeeds in this project's own QEMU setup — see
  the verified transcript in [system/README.md](../README.md). It still fails cleanly
  (timeout, not a crash) with no cable/no server, e.g. on `lo`.
- `WifiManager::scan()` is an honest no-op (see the source comment) since full nl80211
  support is out of scope until there's real wireless hardware to test against.
- BusyBox still provides the same networking applets (`ifconfig`, `route`, ...) — handy
  for independently verifying what `neytra-netup` configured (`busybox ifconfig eth0`,
  `cat /proc/net/route`), not a stopgap anymore now that this module runs at boot.
- `system/init/ServiceManager` does **not** construct `NetworkManager` — `rootfs/init`
  launches the separate `neytra-netup` executable directly (same pattern as `neytra-log`).
- Deferred deliberately: the project's own strategy (root
  [README.md](../../README.md)) says not to start with networking. This is Phase 7 in
  [docs/roadmap.md](../../docs/roadmap.md), after `init`/`shell` are working.
- `DHCPClient`/raw socket access will need privileged operations — expect this to
  eventually depend on [`system/security/`](../security/README.md) (`Sandbox` /
  `PermissionManager`) once that exists.

## See also

- [docs/networking.md](../../docs/networking.md) — full planned design
- [docs/roadmap.md](../../docs/roadmap.md)
