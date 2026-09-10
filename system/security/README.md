# `system/security/` — Security

## Role

Users, permissions, and process sandboxing — the gatekeeping layer any privileged
operation (mounting, networking, package installs) is expected to consult once
implemented.

## Status

**Implemented.** `UserManager` (in-memory user table with passwd-file load/save),
`PermissionManager` (per-uid/resource rule table, root always allowed), and `Sandbox`
(real `fork()` + optional `chroot()`/`setuid()`/rlimits confinement) are all real, built
as `neytra_security`, covered by
[`tests/security/security_test.cpp`](../../tests/security/security_test.cpp).

## Architecture

![Security detailed workflow](design/security-workflow.svg)

`PermissionManager` is the single gatekeeping entry point, consulting `UserManager` for
identity and `Sandbox` for isolation. See
[design/security-workflow.svg](design/security-workflow.svg) for the full step-by-step detail.

## Files

| File | Responsibility (intended) |
|---|---|
| `UserManager.cpp` | User accounts/identities |
| `PermissionManager.cpp` | Access checks against a policy (who may do what) |
| `Sandbox.cpp` | Process isolation/confinement |

## Technical notes

- Follows the interface + constructor-injection pattern from
  [`system/init/`](../init/README.md) (`IUserManager`/`IPermissionManager`/`ISandbox` +
  concrete classes, `ILogger&` injected) — see
  [docs/architecture.md](../../docs/architecture.md#design-principles).
- `Sandbox::run()` only chroots/drops privilege when given a non-empty `chrootDir`/non-zero
  `uid` -- both require running as root; without them it still applies rlimits and is
  fully host-testable (see the test, which never touches chroot/setuid).
- Expected consumers once wired up: `system/process/ProcessManager` (before spawning
  anything privileged) and `system/network/`, `system/package/` (both need privileged
  operations) — none of them call into `system/security/` yet.
- Deferred deliberately: this is Phase 9 in [docs/roadmap.md](../../docs/roadmap.md),
  well after `init`/`shell` are working.

## See also

- [docs/architecture.md](../../docs/architecture.md)
- [docs/roadmap.md](../../docs/roadmap.md)
