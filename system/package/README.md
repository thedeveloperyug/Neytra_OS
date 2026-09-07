# `system/package/` — Package

## Role

Package management: install / remove / upgrade / query software. See
[docs/package-manager.md](../../docs/package-manager.md) for the fuller design
discussion this README summarizes.

## Status

**Stub — earliest stage.** Bare `.cpp` files with only a placeholder comment each and
**no header files yet**.

## Architecture

![Package detailed workflow](design/package-workflow.svg)

`PackageManager` coordinates `Repository` (metadata), `Downloader` (fetch), and
`Installer` (unpack/place). See [design/package-workflow.svg](design/package-workflow.svg)
for the full step-by-step detail.

## Files

| File | Responsibility (intended) |
|---|---|
| `PackageManager.cpp` | Top-level coordinator: install/remove/upgrade/query |
| `Repository.cpp` | Package index/metadata — what's available, versions, dependencies |
| `Downloader.cpp` | Fetches package archives, presumably over `system/network/` |
| `Installer.cpp` | Unpacks and places files on disk, updates local package state |

## Technical notes

- No headers exist yet — when implementing, follow the interface + constructor-injection
  pattern established in [`system/init/`](../init/README.md) (`I<ClassName>.hpp` per
  class, dependencies injected via the constructor, wired together only in a composition
  root) — see [docs/architecture.md](../../docs/architecture.md#design-principles).
- `Downloader` will depend on [`system/network/`](../network/README.md) (also a stub) —
  implement networking first.
- Likely install targets are the already-existing empty directories `usr/`, `opt/`,
  `srv/` under [`rootfs/`](../../rootfs/); [`third_party/`](../../third_party/) hints at
  an intent to vendor some dependencies directly rather than only fetch at runtime.
- Deferred deliberately: this is Phase 7 in [docs/roadmap.md](../../docs/roadmap.md),
  after `init`/`shell` and networking are working.

## See also

- [docs/package-manager.md](../../docs/package-manager.md) — full planned design
- [docs/roadmap.md](../../docs/roadmap.md)
