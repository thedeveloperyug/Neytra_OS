# `system/drivers/` — Drivers

## Role

Low-level peripheral drivers for the Raspberry Pi 4B target (Broadcom BCM2711). These
are only relevant to the [Raspberry Pi path](../../docs/raspberrypi.md) — the x86_64/QEMU
target this project boots today has no use for them.

## Status

**Implemented, but only meaningfully testable on real Raspberry Pi hardware.** `GPIO`
(sysfs `/sys/class/gpio/...`), `I2C` (`/dev/i2c-*` via `ioctl(I2C_SLAVE)`), and `SPI`
(`/dev/spidev*` via `ioctl(SPI_IOC_MESSAGE)`) are real Linux driver-API implementations,
but this project's QEMU x86_64 target has none of that hardware, so calls fail gracefully
-- that's not a bug, [`tests/drivers/drivers_test.cpp`](../../tests/drivers/drivers_test.cpp)
asserts exactly that. `UART` is different: it's plain termios over any character device,
so the test exercises it for real using a pty pair.

## Architecture

![Drivers detailed workflow](design/drivers-workflow.svg)

Four independent peripheral drivers, each with its own open/configure -> transfer ->
consumer flow (no shared parent class). See
[design/drivers-workflow.svg](design/drivers-workflow.svg) for the full per-driver detail.

## Files

| File | Responsibility (intended) |
|---|---|
| `GPIO.cpp` | General-purpose I/O pin access |
| `I2C.cpp` | I2C bus access (sensors, peripherals) |
| `SPI.cpp` | SPI bus access (e.g. displays) |
| `UART.cpp` | Serial port access |

## Technical notes

- Follows the interface pattern from [`system/init/`](../init/README.md)
  (`IGPIO`/`II2C`/`ISPI`/`IUART` + concrete classes) — see
  [docs/architecture.md](../../docs/architecture.md#design-principles). As anticipated,
  the four drivers do **not** share one interface each (design principle #6) — use
  judgement (see design principle #6).
- These map to memory-mapped BCM2711 peripheral registers, not applicable when running
  under QEMU x86_64 — see [docs/raspberrypi.md](../../docs/raspberrypi.md) for the
  current (partial) state of that target: boot files exist, but there's no arm64
  `.config`/cross-compile step wired up yet, so this module has no way to be tested on
  real hardware today either.
- Likely future consumer: [`system/gui/Framebuffer`](../gui/README.md), if the GUI ever
  needs direct display hardware access beyond the kernel's own framebuffer device.

## See also

- [docs/raspberrypi.md](../../docs/raspberrypi.md)
- [docs/roadmap.md](../../docs/roadmap.md)
