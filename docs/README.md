# Neytra OS Documentation

Neytra OS is a from-scratch, educational Linux-based operating system: a real Linux kernel, a hand-built BusyBox root filesystem, a custom PID-1 init script, and a growing custom C++ system layer, all glued together with your own build scripts and booted in QEMU (with Raspberry Pi 4B as a longer-term hardware target).

This directory is the technical documentation for the project. Everything here reflects the **actual state of the repository**, with clear ACTIVE / PARTIAL / PLANNED status markers — nothing is described as done unless it is verified to work.

## Start here

| Doc | What it covers |
|---|---|
| [project-structure.md](project-structure.md) | Full tour of every top-level directory and what it's for |
| [architecture.md](architecture.md) | The layered system architecture, current vs. planned |
| [development-workflow.md](development-workflow.md) | Day-to-day workflow: environment, build, boot, iterate, debug |
| [roadmap.md](roadmap.md) | Phase-by-phase status checklist and what's next |

## Subsystem docs

| Doc | What it covers |
|---|---|
| [boot-process.md](boot-process.md) | Power-on to shell, step by step, for QEMU and Raspberry Pi |
| [kernel-build.md](kernel-build.md) | Fetching, configuring, and building the Linux kernel |
| [rootfs.md](rootfs.md) | Root filesystem layout, BusyBox, init, inittab |
| [qemu-setup.md](qemu-setup.md) | How the OS is tested in QEMU, flags explained |
| [raspberrypi.md](raspberrypi.md) | Raspberry Pi 4B deployment path and its current status |
| [networking.md](networking.md) | Planned networking architecture (not yet implemented) |
| [package-manager.md](package-manager.md) | Planned package manager architecture (not yet implemented) |

## Diagrams

All architecture diagrams are hand-authored SVGs in [diagrams/](diagrams/), referenced from the docs above:

- [diagrams/system-architecture.svg](diagrams/system-architecture.svg) — full layered stack, hardware to applications
- [diagrams/boot-sequence.svg](diagrams/boot-sequence.svg) — power-on to shell, both boot paths
- [diagrams/directory-structure.svg](diagrams/directory-structure.svg) — repository map grouped by purpose
- [diagrams/build-pipeline.svg](diagrams/build-pipeline.svg) — kernel + rootfs build tracks merging into a boot
- [diagrams/init-flow.svg](diagrams/init-flow.svg) — `rootfs/init` flowchart, current and planned
- [diagrams/subsystem-map.svg](diagrams/subsystem-map.svg) — planned `system/` C++ class map

Every diagram uses the same legend: **solid blue** = implemented and active, **dashed amber** = planned/scaffolded only, **green** = upstream Linux kernel, **gray** = hardware or informational.

## Conventions used across these docs

- **ACTIVE** — real, working code you can run today.
- **PARTIAL** — some files/artifacts exist, but the path isn't fully wired up or tested.
- **PLANNED** — intentionally scaffolded for a later phase (per the project's own staged strategy in the root [README.md](../README.md)); usually an empty class or an `echo "... placeholder"` script.

Facts in these docs (versions, file paths, build state) were verified directly against the repository and host tools; see each doc for specifics.
