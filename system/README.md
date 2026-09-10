# `system/` — Neytra OS System Layer

## Role

The self-hosted C++ layer that gradually replaces BusyBox/shell-script scaffolding with
real Neytra code: init, a shell, process management, security, networking, package
management, and Raspberry Pi drivers. See [docs/architecture.md](../docs/architecture.md)
for how this layer fits into the full boot stack, and
[docs/development-workflow.md](../docs/development-workflow.md#8-coding-conventions-observed-in-system)
for the conventions every module below follows (interface + constructor injection, a
`README.md` + `design/*.svg` per module, a matching GTest suite in `tests/`).

## Module status

| Module | Real functionality | Boot-reachable today? |
|---|---|---|
| [`logger/`](logger/README.md) | Leveled console+file logging | ✅ Yes — `rootfs/init` calls `neytra-log` at every boot |
| [`shell/`](shell/README.md) | REPL, 8 builtins, external command spawning | ✅ Yes — run `neytra-shell` manually from the prompt (see below) |
| [`process/`](process/README.md) | fork/exec/wait, nice-value scheduling, FIFO IPC | ✅ Yes — exercised by `neytra-shell` and `neytra-diag` |
| [`network/`](network/README.md) | Sockets, interfaces, DHCP client + real IP/route configuration | ✅ Yes — `rootfs/init` runs `neytra-netup` in the background at every boot (see below) |
| [`security/`](security/README.md) | Users, permission rules, sandboxed exec | 🧪 Diagnostically — `neytra-diag security` (see below); nothing in the OS itself calls it yet |
| [`package/`](package/README.md) | HTTP download + tar install pipeline | 🧪 Diagnostically — `neytra-diag package` (see below); nothing in the OS itself calls it yet |
| [`drivers/`](drivers/README.md) | Raspberry Pi GPIO/I2C/SPI + generic UART | 🧪 Diagnostically — `neytra-diag drivers` (see below); needs real Pi hardware for GPIO/I2C/SPI |
| [`init/`](init/README.md) | Real `mount(2)`, service-spawning from a config file | 🧪 Diagnostically — `neytra-diag init` (see below); `rootfs/init` (shell script) is still real PID 1 |
| [`gui/`](gui/README.md) | — | *Stub, deliberately deferred to last* |

"Boot-reachable" means you can actually exercise the real code path after booting
`scripts/run_qemu_x86.sh`. "Diagnostically" means via the `neytra-diag` CLI, which
exists purely to let you *test* a module by hand — it's not the same as the module being
wired into the OS's own automatic behavior (e.g. `neytra-diag security` proves
`PermissionManager` really works, but nothing else in the OS actually calls
`PermissionManager` yet).

## How to test the services

There are two completely different ways to exercise this code, and they answer different
questions:

### 1. Host-side: `ctest` (the comprehensive way — covers all 8 implemented modules)

This is how every module's *internal correctness* is actually verified — real syscalls,
real sockets, a real forked HTTP server, a real pty, all running on your dev machine, no
QEMU required:

```sh
cmake -S . -B build
cmake --build build -j$(nproc)
cd build && ctest --output-on-failure
```

You should see `100% tests passed, 0 tests failed out of 8`. Run a suite's binary
directly (e.g. `./build/security_test`) for GoogleTest's own colored `[ RUN ]`/`[ OK ]`
per-case output instead of just ctest's one-line summary. See
[docs/development-workflow.md](../docs/development-workflow.md#6-testing) for the full
testing convention.

### 2. Inside a booted VM: `neytra-shell` (the "does it work as a real OS" way)

Today, `neytra-shell` is the **only** module with an interactive entry point that actually
ends up inside the booted rootfs. Here's the full guide:

**Step 1 — Build and boot:**

```sh
bash scripts/build_system.sh    # builds system/, installs neytra-log + neytra-shell + neytra-diag into rootfs/usr/bin/
bash scripts/build_rootfs.sh    # repackages rootfs/ into output/initramfs.cpio.gz
bash scripts/run_qemu_x86.sh    # boots the kernel + initramfs in QEMU
```

**Step 2 — Wait for the BusyBox prompt.** You'll see the boot banner, then:

```
BusyBox v1.36.1 (Ubuntu 1:1.36.1-6ubuntu3.1) built-in shell (ash)
Enter 'help' for a list of built-in commands.

~ #
```

This is still BusyBox's shell (`rootfs/init` hands off to `/bin/sh`) — `neytra-shell`
is *not* the login shell yet, by design (see [shell/README.md](shell/README.md)).

**Step 3 — Run the native shell:**

```
~ # neytra-shell
[19:02:39] [INFO] [Shell] starting interactive session
neytra>
```

**Step 4 — Try it.** Everything below is a real, verified transcript from an actual
QEMU boot (not a hypothetical):

```
neytra> pwd
/
neytra> uname -a
[19:02:51] [INFO] [ProcessManager] spawned uname (pid 72)
Linux (none) 7.1.0-rc3-00005-gc21b90f77687 #1 SMP PREEMPT_DYNAMIC ...
[19:02:51] [INFO] [ProcessManager] pid 72 exited with 0
neytra> export GREETING=hello
neytra> env
NEYTRA_LOG_FILE=/var/log/neytra.log
SHLVL=2
HOME=/
TERM=linux
PATH=/sbin:/usr/sbin:/bin:/usr/bin
PWD=/
GREETING=hello
neytra> help
Neytra shell builtins: cd pwd echo export unset env exit help
neytra> exit 0
[19:03:26] [INFO] [Shell] session ended
~ #
```

`uname -a` above is **not** a builtin — that line proves `neytra-shell` really forked and
executed the real `uname` BusyBox applet through `ProcessManager`, and reaped its real
exit code. Try other real commands the same way: `ls`, `cat <file>`, `ps`, `mkdir`, `rm`,
`dmesg` — anything in `rootfs/bin`/`rootfs/sbin` that BusyBox provides.

**Step 5 — Shut down cleanly:**

```
~ # poweroff -f
```

### What `neytra-shell` can't do (by design, not a bug)

- **No shell grammar**: `CommandParser` is a quote-aware tokenizer, not a POSIX shell
  parser. `ls foo && echo bar`, `cat x | grep y`, `echo hi > file` are all passed as
  literal arguments to the first command, not interpreted as operators. One command per
  line only. See [shell/README.md](shell/README.md).
- **No persisted logging**: `neytra-shell`'s own log lines (`[INFO] [Shell] ...`,
  `[INFO] [ProcessManager] ...`) only go to the console. Unlike `rootfs/init`'s boot
  messages (which set `NEYTRA_LOG_FILE` before calling `neytra-log`), `neytra-shell`
  never calls `setLogFile()`, so none of this ends up in `/var/log/neytra.log`.
- **Builtins only cover the basics**: `cd pwd echo export unset env exit help` — see
  [shell/README.md](shell/README.md) for the exact list.

### 3. Inside a booted VM: `neytra-diag` (test security/network/package/drivers/init)

`neytra-diag` is a second CLI, installed the same way as `neytra-shell`, that drives the
remaining 5 modules for real and prints human-readable results. Run it from the same
BusyBox (or `neytra-shell`) prompt used above:

```
~ # neytra-diag                # runs every section below, in order
~ # neytra-diag security       # UserManager, PermissionManager, Sandbox only
~ # neytra-diag network        # NetworkManager, SocketManager, WifiManager (count only), DHCPClient
~ # neytra-diag wifi           # WifiManager: lists wireless interfaces and scans each one
~ # neytra-diag package        # Repository only (Downloader/Installer need a reachable URL)
~ # neytra-diag drivers        # GPIO, I2C, SPI, UART only
~ # neytra-diag init           # MountManager, ServiceManager only
```

The full list of commands to try after boot, combined:

| Command | Exercises | Needs root? |
|---|---|---|
| `neytra-shell` | `Shell`, `CommandParser`, `BuiltinCommands` | No |
| `neytra-diag security` | `UserManager`, `PermissionManager`, `Sandbox` | No |
| `neytra-diag network` | `NetworkManager`, `SocketManager`, `WifiManager`, `DHCPClient` | Yes, for `setInterfaceUp`/DHCP (you're root by default in this VM) |
| `neytra-diag wifi` | `WifiManager::listWirelessInterfaces()` + `scan()` on each one found | No |
| `neytra-diag package` | `Repository` (index only) | No |
| `neytra-diag drivers` | `GPIO`, `I2C`, `SPI`, `UART` | No (GPIO/I2C/SPI fail gracefully either way without real hardware) |
| `neytra-diag init` | `MountManager`, `ServiceManager` | Yes, for the real `mount(2)` call |

Here's a real, verified transcript from an actual QEMU boot (`neytra-diag all`, edited
only to trim kernel log lines that interleaved on the console):

```
~ # neytra-diag all
== security ==
- root seeded: yes
- users after adding "demo": root(uid=0) demo(uid=1000)
- uid 1000 write /etc/demo: before grant=0 after grant=1
- sandboxed `echo sandboxed-hello` exit code: 0
== network ==
- interface eth0 ip= up=0
- interface lo ip= up=0
- UDP loopback self-test: failed
- wireless interfaces: none (expected, no wifi hardware)
- attempting DHCP on eth0 (up to 3s)...
  leased 10.0.2.15 from gateway 10.0.2.2
== package ==
- Repository loaded "demo" entry: yes, version 1.0
== drivers ==
- GPIO exportPin(999999): failed (expected -- no real GPIO hardware here)
- I2C open(/dev/i2c-0): failed (expected -- no real I2C hardware here)
- SPI open(/dev/spidev0.0): failed (expected -- no real SPI hardware here)
- UART loopback over pty: skipped (openpty failed)
== init ==
- mount tmpfs onto a temp dir: ok
- spawn demo service from config: ok
~ # poweroff -f
```

**The DHCP lease is real** — `DHCPClient` did a genuine DISCOVER→OFFER→ACK exchange
against QEMU's built-in SLIRP DHCP server and got back `10.0.2.15`/gateway `10.0.2.2`,
QEMU usermode networking's well-known default range. Same for the `mount tmpfs` and
service spawn in `init` — both real, because you're root by default in this VM.

### Scanning Wi-Fi from `neytra-shell`

`neytra-shell` has no `wifi` builtin (and won't — see
[shell/README.md](shell/README.md) on keeping builtins generic). Instead, just run
`neytra-diag wifi` as an external command from the `neytra>` prompt, the same way you'd
run `ls` or `uname`. Real, verified transcript:

```
neytra> neytra-diag wifi
[19:32:44] [INFO] [ProcessManager] spawned neytra-diag (pid 73)
== wifi ==
[19:32:44] [INFO] [WifiManager] found 0 wireless interface(s)
- no wireless interfaces found (checked /sys/class/net/*/wireless)
- nothing to scan on this hardware
[19:32:44] [INFO] [ProcessManager] pid 73 exited with 0
neytra>
```

"0 wireless interfaces" is the honest, correct result on this project's QEMU x86_64
target — there's no wifi hardware in the VM at all. On real hardware with a wireless
interface (e.g. a laptop, or eventually a Raspberry Pi with a wifi adapter),
`listWirelessInterfaces()` would find it (confirmed on this repo's host dev machine,
which has one) — but `scan()` itself still won't return real networks yet. It's a
deliberate stub: real scanning needs either the legacy Wireless Extensions ioctls or
full nl80211 generic-netlink support, and implementing that was explicitly deferred
since there's no wireless hardware in this project's actual target (QEMU x86_64 today,
Raspberry Pi eventually) to verify it against — see the comment in
[network/WifiManager.cpp](network/WifiManager.cpp) and
[docs/architecture.md](../docs/architecture.md#design-principles) on avoiding
speculative complexity for untestable code paths.

**Two honest findings from this environment** (not `neytra-diag` bugs):

- **UDP loopback self-test failed.** `rootfs/init` never brings the `lo` interface up,
  so `up=0` for it above — connecting to `127.0.0.1` fails until something runs the
  equivalent of `ip link set lo up`. Nothing does that today.
- **UART test skipped (`openpty` failed).** `openpty()` needs `/dev/pts` (a mounted
  `devpts` filesystem); `rootfs/init` only mounts `proc`/`sysfs`/`devtmpfs`, not
  `devpts`. This is why the exact same `UARTTest` passes on the host (`ctest`) but not
  yet inside the booted VM.

Both are small, known rootfs/init gaps, left as-is here rather than silently patched —
see [docs/roadmap.md](../docs/roadmap.md) if you want to pick either up.

## Automatic network bring-up at boot: `neytra-netup`

Unlike everything above, this one isn't a manually-run diagnostic — `rootfs/init` starts
it **in the background at every boot**, no user action needed. It brings `lo` up, then
for every other real interface it finds: brings it up, waits for the physical link
(polling `/sys/class/net/<name>/carrier`, since an interface can be administratively
"up" before the cable/link has actually negotiated), runs a real DHCP handshake, and if
it gets a lease, **applies it** — assigns the IP/netmask to the interface and installs a
default route via the gateway (`NetworkManager::setInterfaceAddress`/`setDefaultGateway`,
new `ioctl(SIOCSIFADDR/SIOCSIFNETMASK/SIOCADDRT)` calls this session). It never blocks
boot: everything is backgrounded and bounded (5s carrier wait + 8s DHCP timeout per
interface), so a VM/board with no cable just boots normally with a quiet warning logged.

This directly answers "will Neytra OS connect over Ethernet to a router": **the code
path is real, and this transcript is a genuine QEMU boot** (QEMU's default NIC + its
built-in SLIRP DHCP server stand in for "a real router" here — same protocol, same
applied result):

```
[19:43:32] [INFO] [NetworkManager] lo brought up
[19:43:32] [INFO] [init] network bring-up started in background (neytra-netup)
[19:43:32] [INFO] [NetworkManager] eth0 brought up
[19:43:32] [INFO] [init] boot complete, handing off to shell

BusyBox v1.36.1 ... ~ #
[    2.912649] e1000: eth0 NIC Link is Up 1000 Mbps Full Duplex, Flow Control: ...
[19:43:34] [INFO] [DHCPClient] DISCOVER sent on eth0
[19:43:35] [INFO] [DHCPClient] OFFER received
[19:43:35] [INFO] [DHCPClient] ACK received, leased 10.0.2.15
[19:43:35] [INFO] [NetworkManager] eth0 address set to 10.0.2.15/255.255.255.0
[19:43:35] [INFO] [netup] eth0 configured: 10.0.2.15/255.255.255.0
[19:43:35] [INFO] [NetworkManager] default route via 10.0.2.2 on eth0

~ # busybox ifconfig eth0
eth0      Link encap:Ethernet  HWaddr 52:54:00:12:34:56
          inet addr:10.0.2.15  Bcast:10.0.2.255  Mask:255.255.255.0
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
~ # cat /proc/net/route
Iface   Destination     Gateway         Flags ...
eth0    00000000        0202000A        0003  ...   <- default route, really in the kernel
eth0    0002000A        00000000        0001  ...
~ # busybox ping -c 2 10.0.2.2
PING 10.0.2.2 (10.0.2.2): 56 data bytes
64 bytes from 10.0.2.2: seq=0 ttl=255 time=0.230 ms
64 bytes from 10.0.2.2: seq=1 ttl=255 time=0.329 ms
--- 10.0.2.2 ping statistics ---
2 packets transmitted, 2 packets received, 0% packet loss
```

Note the very first attempt (before the carrier-wait fix existed) sent DISCOVER before
the link was actually up and got no reply at all -- worth remembering if you ever see
"no DHCP lease" immediately after boot on a real cable: a slow-negotiating link, not a
broken DHCP client, is the more likely cause.

**What this doesn't answer**: whether this would work on a real Raspberry Pi over real
Ethernet to a real router. The DHCP/IP/route code itself is plain POSIX/ioctl logic with
nothing x86-specific in it, so it should behave the same way once cross-compiled for
ARM64 -- but that ARM64 build doesn't exist yet (see
[docs/raspberrypi.md](../docs/raspberrypi.md)), and the Pi's `bcmgenet` Ethernet driver
needs to be enabled in a kernel config that also doesn't exist yet.

## Why most modules still aren't wired into the OS itself

`security/`, `package/`, `drivers/`, and `init/` are all real, working code — proven by
their own `ctest` suite *and* now by `neytra-diag` above — but nothing in the actual boot
path or `neytra-shell` calls into them as part of normal operation (network/` graduated
out of this list once `neytra-netup` started running automatically). There's no
`pkg`/`useradd`-style builtin wired into `neytra-shell` itself, and `rootfs/init` (not
`system/init/InitManager`) is still real PID 1. This is an honest gap, not an oversight:
per [docs/roadmap.md](../docs/roadmap.md), the next steps are boot-testing
`system/init/` itself as PID 1, and giving `neytra-shell` real builtins that call into
`package/`/`security/` so using them stops requiring a separate diagnostic tool.

## See also

- [docs/architecture.md](../docs/architecture.md) — full layered architecture, design
  principles (dependency injection, composition roots), and the module map.
- [docs/roadmap.md](../docs/roadmap.md) — phase-by-phase status and what's next.
- [docs/development-workflow.md](../docs/development-workflow.md) — build, test, and
  per-module documentation conventions.
- Each module's own `README.md` (linked in the table above) for its real API, design
  diagram, and technical notes.
